#include <neorv32.h>
#define minsum_hw(x)  RISCV_INSTR_R_TYPE(RISCV_OPCODE_CUSTOM0, 0b000, 0b0000000, x, 0)

static void wait_ms(uint32_t ms){ neorv32_aux_delay_ms(neorv32_sysinfo_get_clk(), ms); }

uint32_t pack6(int m[6]){
  uint32_t w=0;
  for(int k=0;k<6;k++){ int s=(m[k]<0); int a=m[k]<0?-m[k]:m[k]; if(a>15)a=15;
    w|=((uint32_t)((s<<4)|(a&0xF)))<<(k*5); }
  return w;
}

int main(void){
  neorv32_rte_setup();
  int msg[6]={3,-5,2,-8,6,-4};
  uint32_t packed=pack6(msg);

  uint32_t r=minsum_hw(packed);
  int correct=(r==0xA32);

  const int LOOP=1000;
  volatile uint32_t sink=0;

  neorv32_cpu_csr_write(CSR_MCYCLE,0);
  for(int n=0;n<LOOP;n++) sink+=minsum_hw(packed);
  uint32_t cfu=neorv32_cpu_csr_read(CSR_MCYCLE);

  neorv32_cpu_csr_write(CSR_MCYCLE,0);
  for(int n=0;n<LOOP;n++){ int a1=99,a2=99,ii=0,ss=0;
    for(int k=0;k<6;k++){ int s=msg[k]<0; ss^=s; int mg=msg[k]<0?-msg[k]:msg[k];
      if(mg<a1){a2=a1;a1=mg;ii=k;} else if(mg<a2)a2=mg; } sink+=a1+a2+ii+ss; }
  uint32_t sw=neorv32_cpu_csr_read(CSR_MCYCLE);
  (void)sink;

  uint32_t sw_pc  = sw / LOOP;     // software cycles per check
  uint32_t cfu_pc = cfu / LOOP;    // CFU cycles per check
  uint32_t ratio  = (cfu>0) ? (sw/cfu) : 0;   // speedup

  while(1){
    if(!correct){ // wrong result -> fast blink
      for(int i=0;i<10;i++){ neorv32_gpio_port_set(0xFF); wait_ms(150);
                             neorv32_gpio_port_set(0x00); wait_ms(150); }
      continue;
    }
    neorv32_gpio_port_set(0x00); wait_ms(4000);           // long dark = restart marker
    neorv32_gpio_port_set(sw_pc  & 0xFF); wait_ms(5000);  // frame 1: SW cycles/check
    neorv32_gpio_port_set(0x00); wait_ms(1500);
    neorv32_gpio_port_set(cfu_pc & 0xFF); wait_ms(5000);  // frame 2: CFU cycles/check
    neorv32_gpio_port_set(0x00); wait_ms(1500);
    neorv32_gpio_port_set(ratio  & 0xFF); wait_ms(5000);  // frame 3: speedup
  }
  return 0;
}