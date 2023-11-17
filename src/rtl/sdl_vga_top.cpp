#include <cstdio>
// #include <cstdlib>
#include <deque>
#include <iostream>
#include <stdlib.h>

#include "./obj_dir/Vvga_top.h"
#include <unistd.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#include "config.h"
#include "vga_top.h"

#include <SDL2/SDL.h>
#define WIDTH 640
#define DEPTH 480
int x = 0;
int y = 0;
bool blank_delay;
bool line_start;

// set dut and c_model macros
#define DUT Vvga_top
#define REF vga_top
#define InIO top_in_io
#define OutIO top_out_io

// #define MAX_SIM_TIME 8
// #define MAX_SIM_TIME 71
// #define MAX_SIM_TIME 202
#define MAX_SIM_TIME 500
// #define MAX_SIM_TIME 2000000
// #define MAX_SIM_TIME 48070
// #define MAX_SIM_TIME 54956
// #define MAX_SIM_TIME 55208
// #define MAX_SIM_TIME 800000
// #define MAX_SIM_TIME 1000000 // this
// #define MAX_SIM_TIME 500000
// #define MAX_SIM_TIME 120000
// #define MAX_SIM_TIME 54696
// #define MAX_SIM_TIME 200000
// #define MAX_SIM_TIME 20000000
uint64_t sim_time;
uint64_t posedge_cnt;

/*
1. get random input for dut and ref
2. connect to dut and ref
*/
class InDriver {
private:
  DUT *dut;
  REF *ref;

public:
  // get random input signal for dut and ref
  void drive(InIO *in) {
    // ppr related data
    dut->arready_i = in->ppr->arready_i;
    dut->rvalid_i = in->ppr->rvalid_i;
    dut->rresp_i = in->ppr->rresp_i;
    dut->rdata_i = in->ppr->rdata_i;
    dut->clk_a = in->ppr->clk_a;
    dut->clk_v = in->ppr->clk_v;
    dut->resetn_a = in->ppr->resetn_a;
    dut->resetn_v = in->ppr->resetn_v;
    // cu related data
    dut->paddr_i = in->cu->paddr_i;
    dut->pwdata_i = in->cu->pwdata_i;
    dut->psel_i = in->cu->psel_i;
    dut->penable_i = in->cu->penable_i;
    dut->pwrite_i = in->cu->pwrite_i;
  }
  // constructor: connect to dut and ref
  InDriver(DUT *d, REF *r) {
    dut = d;
    ref = r;
  }
};
/*
Score Board
1. trace InIO and OutIO
2. Compare dut and c_model
*/
class SCB {

private:
  DUT *dut;
  REF *ref;

public:
  void display() {
    // printf("display dut and ref OutIO at time=%llu\n", sim_time); // llu for
    // mac
    Log("display dut and ref OutIO at time=%lu\n", sim_time);
    Log("red_o    -> dut: 0x%x, ref: 0x%x\n", dut->red_o,
           ref->out->vc->red_o);
    Log("green_o  -> dut: 0x%x, ref: 0x%x\n", dut->green_o,
           ref->out->vc->green_o);
    Log("blue_o   -> dut: 0x%x, ref: 0x%x\n", dut->blue_o,
           ref->out->vc->blue_o);
    Log("hsync_o  -> dut: %d, ref: %d\n", dut->hsync_o,
           ref->out->vc->hsync_o);
    Log("vsync_o  -> dut: %d, ref: %d\n", dut->vsync_o,
           ref->out->vc->vsync_o);
    Log("blank_o  -> dut: %d, ref: %d\n", dut->blank_o,
           ref->out->vc->blank_o);
    // Log("araddr_o -> dut: %llu, ref: %ld\n", dut->araddr_o, // llu for mac
    Log("araddr_o -> dut: %lu, ref: %ld\n", dut->araddr_o,
           ref->out->ppr->araddr_o);
    Log("arburst_o-> dut: %d, ref: %d\n", dut->arburst_o,
           ref->out->ppr->arburst_o);
    Log("arlen_o  -> dut: %d, ref: %d\n", dut->arlen_o,
           ref->out->ppr->arlen_o);
    Log("arsize_o -> dut: %d, ref: %d\n", dut->arsize_o,
           ref->out->ppr->arsize_o);
    Log("arvalid_o-> dut: %d, ref: %d\n", dut->arvalid_o,
           ref->out->ppr->arvalid_o);
    Log("rready_o -> dut: %d, ref: %d\n", dut->rready_o,
           ref->out->ppr->rready_o);
  }
  bool compare() {
    // ref->in->display();
    display();
    // both ppr and vc output content should match
    bool match = dut->araddr_o == ref->out->ppr->araddr_o &&
                 dut->arburst_o == ref->out->ppr->arburst_o &&
                 dut->arlen_o == ref->out->ppr->arlen_o &&
                 dut->arsize_o == ref->out->ppr->arsize_o &&
                 dut->arvalid_o == ref->out->ppr->arvalid_o &&
                 dut->rready_o == ref->out->ppr->rready_o &&
                 dut->red_o == ref->out->vc->red_o &&
                 dut->green_o == ref->out->vc->green_o &&
                 dut->blue_o == ref->out->vc->blue_o &&
                 dut->hsync_o == ref->out->vc->hsync_o &&
                 dut->vsync_o == ref->out->vc->vsync_o &&
                 dut->blank_o == ref->out->vc->blank_o;

    if (match) {
      printf("match\n");
    } else {
      printf("mismatch\n");
    }
    return match;
  }
  // constructor: connect to dut and ref
  SCB(DUT *d, REF *r) {
    dut = d;
    ref = r;
  }
};

/*
1. store InIO into SCB
*/
class InMonitor {};

/*
1. store OutIO to SCB, so SCB can compare dut with ref when necessary
*/
class OutMonitor {

private:
  SCB *scb;
  DUT *dut;
  REF *ref;

public:
  bool monitor_equal() {
    bool equal = scb->compare();
    return equal;
  }

  OutMonitor(SCB *s, DUT *d, REF *r) {
    scb = s;
    dut = d;
    ref = r;
  }
};

// implementations
// declare variables
VerilatedVcdC *m_trace = new VerilatedVcdC;
// Here we create the driver, scoreboard, input and output monitor blocks
DUT *dut = new DUT;
REF *ref = new REF();
InDriver *drv = new InDriver(dut, ref);
SCB *scb = new SCB(dut, ref);
OutMonitor *outMon = new OutMonitor(scb, dut, ref);
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

// init dut, ref and verilator
void init() {
  Log("init\n");
  // init verilator
  Verilated::traceEverOn(true);
  srand(time(NULL));
  dut->trace(m_trace, 0);
  m_trace->open("waveform.vcd");
  sim_time = 0;
  posedge_cnt = 0;
  // init dut
  // init ref
  ref->resetn();
  // init UVM test class
}

// destroy all pointers to free memory
void destroy() {
  Log("destroy\n");
  m_trace->close();
  Log("save waveform\n");
  delete dut;
  // delete m_trace;
  delete outMon;
  delete scb;
  delete drv;
}

// step 1 cycle and compare
void step() {
  Log("step\n");
  while (sim_time < MAX_SIM_TIME) {
    printf("sim_time=%lu, ", sim_time);
    ref->in->randInIO(sim_time);
    drv->drive(ref->in);
    dut->eval(); // dut evaluate
    ref->eval();
    // scb->display();
    // compare dut with ref
    m_trace->dump(sim_time);
    sim_time++;
    if (outMon->monitor_equal() == 0) {
      m_trace->dump(++sim_time);
      destroy();
      _exit(-1);
    };

    // draw SDL renderer
    line_start = (blank_delay == false) && (dut->blank_o == true);
    if (line_start) {
      x = 0;
    } else {
      x = x + 1;
      if (x == 640) {
        y = (y + 1) % 480;
      }
    }
    blank_delay = dut->blank_o;
    if (dut->blank_o) {
      SDL_SetRenderDrawColor(renderer, dut->red_o << 4, dut->green_o << 4,
                             dut->blue_o << 4,
                             255);         // choose another color
      SDL_RenderDrawPoint(renderer, x, y); // draw a pixel use the renderer
    }
  }

  // show SDL contents
  SDL_RenderPresent(renderer);
  SDL_Delay(10000);
}

void init_sdl() {
  SDL_Init(SDL_INIT_VIDEO);
  // SDL_CreateWindowAndRenderer(WIDTH, DEPTH, 0, &window, &renderer);
  SDL_CreateWindowAndRenderer(WIDTH * 4, DEPTH * 4, 0, &window, &renderer);
  SDL_RenderSetScale(renderer, 4, 4);
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // set renderer color
  SDL_RenderClear(renderer); // clear the screen with the renderer color
}

int main(int argc, char **argv) {
  // init dut, ref and verilator
  init();

  init_sdl();
  // step and compare
  step();
  // destroy pointers
  destroy();
}