/* Min-sum LDPC decoder reference model + fixed-point quantisation study.
 *
 * Builds a small regular (3,6) rate-1/2 LDPC code, transmits over an AWGN
 * channel, and decodes with the normalized min-sum algorithm. Reports BER
 * and average iteration count for a 32-bit float datapath and for Q-bit
 * fixed-point datapaths, matching the hardware study.
 *
 * Build:  make         Run:  ./ldpc_minsum
 *
 * NOTE: this is the PC reference used to validate the algorithm and to pick
 * the datapath width. The on-FPGA decoder is in ../../firmware/cfu_minsum.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---- Code parameters (regular (3,6), rate 1/2) ---- */
#define N        128          /* codeword length (variable nodes) */
#define M        64           /* parity checks  (check nodes)     */
#define DV       3            /* variable node degree             */
#define DC       6            /* check node degree                */
#define MAXIT    50           /* max decoding iterations          */
#define FRAMES   15000        /* Monte-Carlo frames per SNR point */

/* Sparse parity-check matrix H, stored as node adjacency lists. */
static int chk_of_var[N][DV];   /* checks connected to each variable node */
static int var_of_chk[M][DC];   /* variables connected to each check node */

/* Build a simple regular (3,6) H by random edge permutation (girth not
 * optimised; good enough for a reference model). Deterministic via seed. */
static void build_H(unsigned seed) {
    srand(seed);
    int edges = N * DV;                 /* = M * DC */
    int *sockets = malloc(sizeof(int) * edges);
    for (int e = 0; e < edges; e++) sockets[e] = e % M;   /* check sockets */
    /* Fisher-Yates shuffle */
    for (int e = edges - 1; e > 0; e--) {
        int j = rand() % (e + 1);
        int t = sockets[e]; sockets[e] = sockets[j]; sockets[j] = t;
    }
    int cdeg[M]; memset(cdeg, 0, sizeof(cdeg));
    int e = 0;
    for (int v = 0; v < N; v++)
        for (int k = 0; k < DV; k++) {
            int c = sockets[e++];
            chk_of_var[v][k] = c;
            if (cdeg[c] < DC) var_of_chk[c][cdeg[c]++] = v;
        }
    free(sockets);
}

/* Gaussian noise (Box-Muller). */
static double randn(void) {
    double u1 = (rand() + 1.0) / (RAND_MAX + 2.0);
    double u2 = (rand() + 1.0) / (RAND_MAX + 2.0);
    return sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
}

/* Quantise an LLR to Q bits (signed, symmetric clip). Q<=0 => float. */
static double quantise(double x, int Q) {
    if (Q <= 0) return x;
    double lim = (double)((1 << (Q - 1)) - 1);   /* e.g. Q=6 -> +/-31 */
    double step = 8.0 / lim;                      /* map +/-8.0 LLR range */
    double q = round(x / step);
    if (q >  lim) q =  lim;
    if (q < -lim) q = -lim;
    return q * step;
}

/* One frame: all-zero codeword (BPSK +1), AWGN, min-sum decode.
 * Returns bit errors; *iters set to iterations used. */
static int decode_frame(double sigma, int Q, int *iters) {
    double ch[N], Lq[N][DV], Lr[M][DC];
    for (int v = 0; v < N; v++) {
        double y = 1.0 + sigma * randn();         /* +1 BPSK + noise */
        ch[v] = quantise(2.0 * y / (sigma * sigma), Q);
        for (int k = 0; k < DV; k++) Lq[v][k] = ch[v];
    }
    int it;
    for (it = 1; it <= MAXIT; it++) {
        /* check-node update: normalized min-sum */
        for (int c = 0; c < M; c++) {
            for (int a = 0; a < DC; a++) {
                int sign = 1; double min1 = 1e9;
                for (int b = 0; b < DC; b++) {
                    if (b == a) continue;
                    int v = var_of_chk[c][b];
                    /* find the message v->c */
                    double msg = 0;
                    for (int k = 0; k < DV; k++) if (chk_of_var[v][k] == c) { msg = Lq[v][k]; break; }
                    if (msg < 0) sign = -sign;
                    double mag = fabs(msg);
                    if (mag < min1) min1 = mag;
                }
                Lr[c][a] = quantise(0.75 * sign * min1, Q);   /* 0.75 = scaling */
            }
        }
        /* variable-node update + hard decision + syndrome */
        int bad = 0;
        double post[N];
        for (int v = 0; v < N; v++) {
            double sum = ch[v];
            for (int k = 0; k < DV; k++) {
                int c = chk_of_var[v][k];
                for (int a = 0; a < DC; a++) if (var_of_chk[c][a] == v) { sum += Lr[c][a]; break; }
            }
            post[v] = sum;
            for (int k = 0; k < DV; k++) {
                int c = chk_of_var[v][k];
                double sub = 0;
                for (int a = 0; a < DC; a++) if (var_of_chk[c][a] == v) { sub = Lr[c][a]; break; }
                Lq[v][k] = quantise(sum - sub, Q);
            }
        }
        /* syndrome check (all-zero codeword => decoded bit = post<0) */
        for (int c = 0; c < M; c++) {
            int par = 0;
            for (int a = 0; a < DC; a++) par ^= (post[var_of_chk[c][a]] < 0);
            if (par) { bad = 1; break; }
        }
        if (!bad) break;
    }
    if (it > MAXIT) it = MAXIT;
    *iters = it;
    int errs = 0;
    /* recompute final decision */
    for (int v = 0; v < N; v++) {
        double sum = ch[v];
        for (int k = 0; k < DV; k++) {
            int c = chk_of_var[v][k];
            for (int a = 0; a < DC; a++) if (var_of_chk[c][a] == v) { sum += Lr[c][a]; break; }
        }
        if (sum < 0) errs++;   /* transmitted all zeros */
    }
    return errs;
}

static void run_point(double EbN0_dB, int Q) {
    double rate = 0.5;
    double EbN0 = pow(10.0, EbN0_dB / 10.0);
    double sigma = sqrt(1.0 / (2.0 * rate * EbN0));
    long bit_errs = 0, bits = 0, it_sum = 0;
    for (int f = 0; f < FRAMES; f++) {
        int iters;
        bit_errs += decode_frame(sigma, Q, &iters);
        it_sum += iters; bits += N;
    }
    printf("  %-14s BER = %.2e   avg_iters = %.2f\n",
           Q <= 0 ? "32-bit float" : (Q == 6 ? "6-bit fixed" : "5-bit fixed"),
           (double)bit_errs / bits, (double)it_sum / FRAMES);
}

int main(void) {
    build_H(1);
    printf("Length-%d rate-1/2 LDPC, min-sum, up to %d iters, %d frames/point\n\n",
           N, MAXIT, FRAMES);
    for (double snr = 3.0; snr <= 5.0; snr += 1.0) {
        printf("=== Eb/N0 = %.0f dB ===\n", snr);
        run_point(snr, 0);    /* float */
        run_point(snr, 6);    /* 6-bit */
        run_point(snr, 5);    /* 5-bit */
        printf("\n");
    }
    return 0;
}
