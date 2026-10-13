#!/usr/bin/env python3
"""Generate the BER waterfall and convergence figures for the report/paper.

By default this uses the illustrative measured points from the project. To plot
your own numbers, edit the DATA dictionaries or parse the output of
../reference/ldpc_minsum.
"""
import matplotlib.pyplot as plt

# Eb/N0 (dB) -> BER, for each datapath. Replace with your measured values.
SNR = [2.5, 3.0, 3.5, 4.0, 4.5]
BER = {
    "32-bit float": [1.2e-2, 4.9e-3, 1.4e-3, 3.0e-4, 3.1e-5],
    "6-bit":        [1.2e-2, 4.7e-3, 1.4e-3, 2.9e-4, 2.6e-5],
    "5-bit":        [1.3e-2, 4.8e-3, 1.5e-3, 3.1e-4, 3.0e-5],
    "4-bit":        [1.6e-2, 7.0e-3, 2.3e-3, 6.0e-4, 9.0e-5],
    "3-bit":        [3.0e-2, 1.8e-2, 9.0e-3, 3.5e-3, 1.1e-3],
}

def waterfall(keys, fname, title):
    plt.figure(figsize=(5.2, 4.0))
    for k in keys:
        plt.semilogy(SNR, BER[k], marker="o", label=k)
    plt.xlabel(r"$E_b/N_0$ (dB)")
    plt.ylabel("Bit Error Rate")
    plt.title(title)
    plt.grid(True, which="both", ls=":")
    plt.legend()
    plt.tight_layout()
    plt.savefig(fname, dpi=150)
    print("wrote", fname)

# Average iterations vs SNR (convergence)
SNR_C = [0, 1, 2, 3, 4, 5, 6, 7, 8]
ITERS = [6.4, 5.2, 3.9, 2.8, 2.1, 1.6, 1.3, 1.1, 1.0]

def convergence(fname):
    plt.figure(figsize=(5.2, 3.8))
    plt.plot(SNR_C, ITERS, marker="s", color="#1C7293")
    plt.xlabel(r"$E_b/N_0$ (dB)")
    plt.ylabel("Average iterations")
    plt.title("Decoding convergence")
    plt.grid(True, ls=":")
    plt.tight_layout()
    plt.savefig(fname, dpi=150)
    print("wrote", fname)

if __name__ == "__main__":
    waterfall(["32-bit float", "6-bit", "5-bit"], "ber_high.png",
              "Quantisation vs float (higher SNR)")
    waterfall(["32-bit float", "4-bit", "3-bit"], "ber_low.png",
              "Quantisation vs float (lower SNR)")
    convergence("convergence.png")
