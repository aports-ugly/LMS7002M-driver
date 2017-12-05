///
/// \file LMS7002M_filter_cal.c
///
/// Common filter calibration functions for the LMS7002M C driver.
///
/// \copyright
/// Copyright (c) 2016-2017 Fairwaves, Inc.
/// Copyright (c) 2016-2016 Rice University
/// SPDX-License-Identifier: Apache-2.0
/// http://www.apache.org/licenses/LICENSE-2.0
///

#include "LMS7002M_impl.h"
#include "LMS7002M_filter_cal.h"
#include <LMS7002M/LMS7002M_logger.h>
#include <LMS7002M/LMS7002M_time.h>

static long long cal_rssi_sleep_ticks(void)
{
    return (LMS7_time_tps())/1000; //1 ms -> ticks
}

uint16_t cal_read_rssi(LMS7002M_t *self, const LMS7002M_chan_t channel)
{
    const unsigned N = 4;
    const unsigned S = 1;
    unsigned i;
    int rssi_v[N];
    int rssi_avg = 0;

    LMS7_sleep_for(cal_rssi_sleep_ticks());

    for (i = 0; i < N; i++)
    {
        LMS7_sleep_for(cal_rssi_sleep_ticks()/4);

        rssi_v[i] = LMS7002M_rxtsp_read_rssi(self, channel);
        if (i >= S)
            rssi_avg += rssi_v[i];
    }
#if 0
    LMS7_logf(LMS7_FATAL, self, "RSSI: %d [%d %d %d %d  %d %d %d %d  %d %d %d %d  %d %d %d %d ]",
              2 * rssi_avg / N,
              rssi_v[0], rssi_v[1], rssi_v[2], rssi_v[3],
            rssi_v[4], rssi_v[5], rssi_v[6], rssi_v[7],
            rssi_v[8], rssi_v[9], rssi_v[10], rssi_v[11],
            rssi_v[12], rssi_v[13], rssi_v[14], rssi_v[15]);
#else
    LMS7_logf(LMS7_FATAL, self, "RSSI: %d [%d %d %d %d]",
              rssi_avg / (N - S),
              rssi_v[0], rssi_v[1], rssi_v[2], rssi_v[3]);
#endif


    return rssi_avg / (N - S);
}

void set_addrs_to_default(LMS7002M_t *self, const LMS7002M_chan_t channel, const int start_addr, const int stop_addr)
{
    LMS7002M_set_mac_ch(self, channel);
    for (int addr = start_addr; addr <= stop_addr; addr++)
    {
        int value = LMS7002M_regs_default(addr);
        if (value == -1) continue; //not in map
        LMS7002M_regs_set(LMS7002M_regs(self), addr, value);
        LMS7002M_regs_spi_write(self, addr);
    }
}

int cal_gain_selection(LMS7002M_t *self, const LMS7002M_chan_t channel, int rssi_level)
{
    // TODO find optimal PGA & TBB ( & RFE loopback ) that don't saturate
    // RX and TX
    int rssi_max;
    int rssi;

    while (true)
    {
#if 0
        const int rssi_value_50k = cal_read_rssi(self, channel);

        LMS7_logf(LMS7_FATAL, self, "RSSI: %d TBB: %d PGA: %d",
                  rssi_value_50k,
                  LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb,
                  LMS7002M_regs(self)->reg_0x0119_g_pga_rbb);

        if (rssi_value_50k > 0x8400) break;

        LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb++;
        if (LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb > 63)
        {
            if (LMS7002M_regs(self)->reg_0x0119_g_pga_rbb > 31) break;
            LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb = 1;
            LMS7002M_regs(self)->reg_0x0119_g_pga_rbb += 6;
        }

        LMS7002M_regs_spi_write(self, 0x0108);
        LMS7002M_regs_spi_write(self, 0x0119);
#else
        int best_lo = -1; //int rssi_lo = -1;
        int best_hi = -1; //int rssi_hi = -1;

        int range = LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb = 32;
        int rssi_value_50k;

        rssi_max = 0;

        do
        {
            int old_cg_iamp_tbb = LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb;
            LMS7002M_regs_spi_write(self, 0x0108);

            rssi_value_50k = cal_read_rssi(self, channel);
            if (rssi_value_50k > rssi_max)
                rssi_max = rssi_value_50k;

            if (rssi_value_50k > rssi_level/*0.8 * 65536*/)
            {
                best_hi = LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb;
                //rssi_hi = rssi_value_50k;
                LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb -= range/2;
            }
            else
            {
                best_lo = LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb;
                //rssi_lo = rssi_value_50k;
                LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb += range/2;
            }

            LMS7_logf(LMS7_FATAL, self, "RSSI: %d -- %d (range %d)",
                      rssi_value_50k,
                      old_cg_iamp_tbb,
                      range);
            range /= 2;
        }
        while (range != 0);

        if (best_hi != -1)
        {
            if (best_lo != -1)
            {
                LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb = best_lo;
                LMS7002M_regs_spi_write(self, 0x0108);
            }
            else if (LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb != best_hi)
            {
                LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb = best_hi;
                LMS7002M_regs_spi_write(self, 0x0108);
            }
            break;
        }

        if (LMS7002M_regs(self)->reg_0x0119_g_pga_rbb == 31)
        {
            // unable to tune!
            break;
        }

        // Not enough gain in the loopback
        LMS7002M_regs(self)->reg_0x0119_g_pga_rbb += 6;
        if (LMS7002M_regs(self)->reg_0x0119_g_pga_rbb > 31)
        {
            LMS7002M_regs(self)->reg_0x0119_g_pga_rbb = 31;
        }
        LMS7002M_regs_spi_write(self, 0x0119);
#endif
    }

    rssi = cal_read_rssi(self, channel);
    LMS7_logf(LMS7_FATAL, self, "RSSI: %d (MAX %d) TBB: %d PGA: %d",
              rssi, rssi_max,
              LMS7002M_regs(self)->reg_0x0108_cg_iamp_tbb,
              LMS7002M_regs(self)->reg_0x0119_g_pga_rbb);
    return rssi;
}

int cal_setup_cgen(LMS7002M_t *self, const double bw)
{
    double cgen_freq = bw*20;
    if (cgen_freq < 60e6) cgen_freq = 60e6;
    if (cgen_freq > 640e6) cgen_freq = 640e6;
    while ((int)(cgen_freq/1e6) == (int)(bw/16e6)) cgen_freq -= 10e6;
    return LMS7002M_set_data_clock(self, self->cgen_fref, cgen_freq, NULL);
}
