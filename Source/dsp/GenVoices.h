// TRoss 606 drum voices.
// Contains code derived from 606-Inspired-Synth-Drums, (c) 2026 Matthew Fecher, MIT licence
// (see LICENSE-606-Inspired-Synth-Drums.txt).
#pragma once
#include <cmath>
#include <cstdint>
#include <cstddef>
#include <cstring>

#if defined (__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wfloat-equal"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wunused-variable"
 #pragma clang diagnostic ignored "-Wunused-but-set-variable"
#elif defined (__GNUC__)
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wfloat-equal"
 #pragma GCC diagnostic ignored "-Wsign-conversion"
 #pragma GCC diagnostic ignored "-Wshadow"
 #pragma GCC diagnostic ignored "-Wunused-variable"
 #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#elif defined (_MSC_VER)
 #pragma warning (push)
 #pragma warning (disable: 4100 4189 4244 4456 4457 4458)
#endif

namespace tross606 {

static inline double gmin(double a, double b) { return a < b ? a : b; }
static inline double gmax(double a, double b) { return a > b ? a : b; }
static inline double gclamp(double x, double lo, double hi) { return x < lo ? lo : (x > hi ? hi : x); }

static inline double dcoef(double a_fs, double a_s) {
  return std::exp(-1.0 / (a_fs * gmax(0.0005, a_s)));
}
static inline double opcoef(double a_fs, double a_s) {
  return std::exp(-1.0 / (a_fs * gmax(0.00001, a_s)));
}
static inline double t60coef(double a_fs, double a_s) {
  return std::exp(-6.9077553 / (a_fs * gmax(0.0001, a_s)));
}
static inline double lpcoef(double a_fs, double a_f) {
  return 1.0 - std::exp(-6.283185307179586 * gclamp(a_f, 10.0, a_fs * 0.45) / a_fs);
}
static inline double wrap2pi(double a_x) {
  return a_x - 6.283185307179586 * std::floor(a_x / 6.283185307179586);
}
static inline double velgain(double a_v, double a_acc) {
    double vg_g = 0.0;
  vg_g = 0.3 + 0.7 * gmin(a_v, 100.0) / 100.0;
  if (a_v > 100.0) {
    vg_g = vg_g * (1.0 + a_acc);
  }
  return vg_g;
}
static inline double lvlgain(double a_db) {
    double lg_g = 0.0;
  lg_g = 0.0;
  if (a_db > -69.9) {
    lg_g = std::pow(10.0, a_db / 20.0);
  }
  return lg_g;
}
static inline double rsg(double a_t, double a_on, double a_att, double a_ho, double a_t60) {
    double rs_ag = 0.0, rs_age = 0.0, rs_da = 0.0, rs_out = 0.0, rs_sn = 0.0;
  rs_age = a_t - a_on;
  rs_out = 0.0;
  if (rs_age >= 0.0) {
    rs_ag = 1.0;
    if (a_att > 0.0 && rs_age < a_att) {
      rs_sn = std::sin(0.5 * 3.141592653589793 * rs_age / a_att);
      rs_ag = rs_sn * rs_sn;
    }
    rs_da = gmax(rs_age - a_att - a_ho, 0.0);
    rs_out = rs_ag * std::exp(-6.9077553 * rs_da / gmax(a_t60, 0.000001));
  }
  return rs_out;
}
static inline double blep(double a_t, double a_dt) {
    double bl_out = 0.0, bl_x = 0.0;
  bl_out = 0.0;
  if (a_t < a_dt) {
    bl_x = a_t / a_dt;
    bl_out = bl_x + bl_x - bl_x * bl_x - 1.0;
  }
  else if (a_t > 1.0 - a_dt) {
    bl_x = (a_t - 1.0) / a_dt;
    bl_out = bl_x * bl_x + bl_x + bl_x + 1.0;
  }
  return bl_out;
}
static inline double rcr(double a_t, double a_s, double a_e) {
    double rc_out = 0.0;
  rc_out = 1.0;
  if (a_t <= a_s) {
    rc_out = 0.0;
  }
  else if (a_t < a_e) {
    rc_out = 0.5 - 0.5 * std::cos(3.141592653589793 * (a_t - a_s) / (a_e - a_s));
  }
  return rc_out;
}

struct GenVoices {
    // ---- Params (set from the plug-in)
    double master = -6.0;
    double accent = 50.0;
    double bd_lvl = 0.0;
    double bd_tune = 2.2;
    double bd_decay = 80.0;
    double bd_attack = 30.0;
    double sd_lvl = 0.0;
    double sd_tune = 0.0;
    double sd_decay = 80.0;
    double sd_snappy = 75.0;
    double lt_lvl = 0.0;
    double lt_tune = 0.0;
    double lt_decay = 80.0;
    double ht_lvl = 0.0;
    double ht_tune = 0.0;
    double ht_decay = 80.0;
    double cp_lvl = 0.0;
    double cp_tune = 0.0;
    double cp_decay = 80.0;
    double cp_noise = 50.0;
    double ch_lvl = 0.0;
    double ch_tune = 0.0;
    double ch_decay = 70.0;
    double oh_lvl = 0.0;
    double oh_tune = 0.0;
    double oh_decay = 70.0;
    double cy_lvl = 0.0;
    double cy_tune = 0.0;
    double cy_decay = 70.0;
    double cy_tone = 50.0;
    double tr_bd = 0.0;
    double tr_sd = 0.0;
    double tr_lt = 0.0;
    double tr_ht = 0.0;
    double tr_cp = 0.0;
    double tr_ch = 0.0;
    double tr_oh = 0.0;
    double tr_cy = 0.0;
    double seqon = 1.0;
    double mute_bd = 0.0;
    double solo_bd = 0.0;
    double mute_sd = 0.0;
    double solo_sd = 0.0;
    double mute_lt = 0.0;
    double solo_lt = 0.0;
    double mute_ht = 0.0;
    double solo_ht = 0.0;
    double mute_cp = 0.0;
    double solo_cp = 0.0;
    double mute_ch = 0.0;
    double solo_ch = 0.0;
    double mute_oh = 0.0;
    double solo_oh = 0.0;
    double mute_cy = 0.0;
    double solo_cy = 0.0;
    double stp = 0.0;
    double seqtick = 0.0;
    double m_ac = 0.0;
    double m_bd = 0.0;
    double m_sd = 0.0;
    double m_lt = 0.0;
    double m_ht = 0.0;
    double m_cp = 0.0;
    double m_ch = 0.0;
    double m_oh = 0.0;
    double m_cy = 0.0;

    // ---- History
    double h_ticklast = 0.0;
    double h_bd_mg = 1.0;
    double h_sd_mg = 1.0;
    double h_lt_mg = 1.0;
    double h_ht_mg = 1.0;
    double h_cp_mg = 1.0;
    double h_ch_mg = 1.0;
    double h_oh_mg = 1.0;
    double h_cy_mg = 1.0;
    double h_bd_active = 0.0;
    double h_bd_trlast = 0.0;
    double h_bd_vg = 1.0;
    double h_bd_sil = 0.0;
    double h_bd_ph = 0.0;
    double h_bd_hz = 100.0;
    double h_bd_tg = 100.0;
    double h_bd_amp = 0.0;
    double h_bd_ampc = 0.999;
    double h_bd_pc = 0.001;
    double h_bd_ce = 0.0;
    double h_bd_cc = 0.999;
    double h_bd_ie = 0.0;
    double h_bd_ic = 0.999;
    double h_bd_clz = 0.0;
    double h_bd_cla = 0.1;
    double h_bd_blz = 0.0;
    double h_bd_bla = 0.1;
    double h_bd_ilz = 0.0;
    double h_bd_ila = 0.1;
    double h_bd_dcx = 0.0;
    double h_bd_dcy = 0.0;
    double h_bd_last = 0.0;
    double h_bd_nin = 0.0;
    double h_bd_nout = 0.0;
    double h_bd_amt = 0.25;
    double h_bd_hp_b0 = 1.0;
    double h_bd_hp_b1 = 0.0;
    double h_bd_hp_b2 = 0.0;
    double h_bd_hp_a1 = 0.0;
    double h_bd_hp_a2 = 0.0;
    double h_bd_hp_z1 = 0.0;
    double h_bd_hp_z2 = 0.0;
    double h_sd_active = 0.0;
    double h_sd_trlast = 0.0;
    double h_sd_vg = 1.0;
    double h_sd_fi = 0.0;
    double h_sd_nf = 0.0;
    double h_sd_bph = 0.0;
    double h_sd_rph = 0.0;
    double h_sd_dec = 1.0;
    double h_sd_pr = 1.0;
    double h_sd_snap = 1.0;
    double h_sd_col = 1.0;
    double h_sd_sel = 0.370136054422;
    double h_sd_ghold = 0.370136054422;
    double h_sd_gfade = 0.0;
    double h_sd_ngain = 1.0;
    double h_sd_lpz = 0.0;
    double h_sd_lpa = 0.1;
    double h_sd_b1_b0 = 1.0;
    double h_sd_b1_b1 = 0.0;
    double h_sd_b1_b2 = 0.0;
    double h_sd_b1_a1 = 0.0;
    double h_sd_b1_a2 = 0.0;
    double h_sd_b1_z1 = 0.0;
    double h_sd_b1_z2 = 0.0;
    double h_sd_b2_b0 = 1.0;
    double h_sd_b2_b1 = 0.0;
    double h_sd_b2_b2 = 0.0;
    double h_sd_b2_a1 = 0.0;
    double h_sd_b2_a2 = 0.0;
    double h_sd_b2_z1 = 0.0;
    double h_sd_b2_z2 = 0.0;
    double h_lt_active = 0.0;
    double h_lt_trlast = 0.0;
    double h_lt_vg = 1.0;
    double h_lt_mhz = 0.0;
    double h_lt_glide = 0.0;
    double h_lt_gpole = 0.0;
    double h_lt_lhz = 0.0;
    double h_lt_shz = 0.0;
    double h_lt_mph = 0.0;
    double h_lt_lph = 0.0;
    double h_lt_sph = 0.0;
    double h_lt_me = 0.0;
    double h_lt_le = 0.0;
    double h_lt_se = 0.0;
    double h_lt_sne = 0.0;
    double h_lt_bue = 0.0;
    double h_lt_tne = 0.0;
    double h_lt_mpole = 0.0;
    double h_lt_lpole = 0.0;
    double h_lt_spole = 0.0;
    double h_lt_snp = 0.0;
    double h_lt_bup = 0.0;
    double h_lt_tnp = 0.0;
    double h_lt_br = 0.0;
    double h_lt_brc = 0.0;
    double h_lt_sr = 0.0;
    double h_lt_src = 0.0;
    double h_lt_snr = 0.0;
    double h_lt_snrc = 0.0;
    double h_lt_nr = 0.0;
    double h_lt_nrc = 0.0;
    double h_lt_lnh_b0 = 1.0;
    double h_lt_lnh_b1 = 0.0;
    double h_lt_lnh_b2 = 0.0;
    double h_lt_lnh_a1 = 0.0;
    double h_lt_lnh_a2 = 0.0;
    double h_lt_lnh_z1 = 0.0;
    double h_lt_lnh_z2 = 0.0;
    double h_lt_lnl_b0 = 1.0;
    double h_lt_lnl_b1 = 0.0;
    double h_lt_lnl_b2 = 0.0;
    double h_lt_lnl_a1 = 0.0;
    double h_lt_lnl_a2 = 0.0;
    double h_lt_lnl_z1 = 0.0;
    double h_lt_lnl_z2 = 0.0;
    double h_lt_hnh_b0 = 1.0;
    double h_lt_hnh_b1 = 0.0;
    double h_lt_hnh_b2 = 0.0;
    double h_lt_hnh_a1 = 0.0;
    double h_lt_hnh_a2 = 0.0;
    double h_lt_hnh_z1 = 0.0;
    double h_lt_hnh_z2 = 0.0;
    double h_lt_hnl_b0 = 1.0;
    double h_lt_hnl_b1 = 0.0;
    double h_lt_hnl_b2 = 0.0;
    double h_lt_hnl_a1 = 0.0;
    double h_lt_hnl_a2 = 0.0;
    double h_lt_hnl_z1 = 0.0;
    double h_lt_hnl_z2 = 0.0;
    double h_lt_tnh_b0 = 1.0;
    double h_lt_tnh_b1 = 0.0;
    double h_lt_tnh_b2 = 0.0;
    double h_lt_tnh_a1 = 0.0;
    double h_lt_tnh_a2 = 0.0;
    double h_lt_tnh_z1 = 0.0;
    double h_lt_tnh_z2 = 0.0;
    double h_lt_tnl_b0 = 1.0;
    double h_lt_tnl_b1 = 0.0;
    double h_lt_tnl_b2 = 0.0;
    double h_lt_tnl_a1 = 0.0;
    double h_lt_tnl_a2 = 0.0;
    double h_lt_tnl_z1 = 0.0;
    double h_lt_tnl_z2 = 0.0;
    double h_lt_fsh_b0 = 1.0;
    double h_lt_fsh_b1 = 0.0;
    double h_lt_fsh_b2 = 0.0;
    double h_lt_fsh_a1 = 0.0;
    double h_lt_fsh_a2 = 0.0;
    double h_lt_fsh_z1 = 0.0;
    double h_lt_fsh_z2 = 0.0;
    double h_lt_fsl_b0 = 1.0;
    double h_lt_fsl_b1 = 0.0;
    double h_lt_fsl_b2 = 0.0;
    double h_lt_fsl_a1 = 0.0;
    double h_lt_fsl_a2 = 0.0;
    double h_lt_fsl_z1 = 0.0;
    double h_lt_fsl_z2 = 0.0;
    double h_ht_active = 0.0;
    double h_ht_trlast = 0.0;
    double h_ht_vg = 1.0;
    double h_ht_mhz = 0.0;
    double h_ht_glide = 0.0;
    double h_ht_gpole = 0.0;
    double h_ht_lhz = 0.0;
    double h_ht_shz = 0.0;
    double h_ht_mph = 0.0;
    double h_ht_lph = 0.0;
    double h_ht_sph = 0.0;
    double h_ht_me = 0.0;
    double h_ht_le = 0.0;
    double h_ht_se = 0.0;
    double h_ht_sne = 0.0;
    double h_ht_bue = 0.0;
    double h_ht_tne = 0.0;
    double h_ht_mpole = 0.0;
    double h_ht_lpole = 0.0;
    double h_ht_spole = 0.0;
    double h_ht_snp = 0.0;
    double h_ht_bup = 0.0;
    double h_ht_tnp = 0.0;
    double h_ht_br = 0.0;
    double h_ht_brc = 0.0;
    double h_ht_sr = 0.0;
    double h_ht_src = 0.0;
    double h_ht_snr = 0.0;
    double h_ht_snrc = 0.0;
    double h_ht_nr = 0.0;
    double h_ht_nrc = 0.0;
    double h_ht_uph0 = 0.0;
    double h_ht_uhz0 = 0.0;
    double h_ht_ue0 = 0.0;
    double h_ht_up0 = 0.0;
    double h_ht_uph1 = 0.0;
    double h_ht_uhz1 = 0.0;
    double h_ht_ue1 = 0.0;
    double h_ht_up1 = 0.0;
    double h_ht_uph2 = 0.0;
    double h_ht_uhz2 = 0.0;
    double h_ht_ue2 = 0.0;
    double h_ht_up2 = 0.0;
    double h_ht_uph3 = 0.0;
    double h_ht_uhz3 = 0.0;
    double h_ht_ue3 = 0.0;
    double h_ht_up3 = 0.0;
    double h_ht_lnh_b0 = 1.0;
    double h_ht_lnh_b1 = 0.0;
    double h_ht_lnh_b2 = 0.0;
    double h_ht_lnh_a1 = 0.0;
    double h_ht_lnh_a2 = 0.0;
    double h_ht_lnh_z1 = 0.0;
    double h_ht_lnh_z2 = 0.0;
    double h_ht_lnl_b0 = 1.0;
    double h_ht_lnl_b1 = 0.0;
    double h_ht_lnl_b2 = 0.0;
    double h_ht_lnl_a1 = 0.0;
    double h_ht_lnl_a2 = 0.0;
    double h_ht_lnl_z1 = 0.0;
    double h_ht_lnl_z2 = 0.0;
    double h_ht_hnh_b0 = 1.0;
    double h_ht_hnh_b1 = 0.0;
    double h_ht_hnh_b2 = 0.0;
    double h_ht_hnh_a1 = 0.0;
    double h_ht_hnh_a2 = 0.0;
    double h_ht_hnh_z1 = 0.0;
    double h_ht_hnh_z2 = 0.0;
    double h_ht_hnl_b0 = 1.0;
    double h_ht_hnl_b1 = 0.0;
    double h_ht_hnl_b2 = 0.0;
    double h_ht_hnl_a1 = 0.0;
    double h_ht_hnl_a2 = 0.0;
    double h_ht_hnl_z1 = 0.0;
    double h_ht_hnl_z2 = 0.0;
    double h_ht_tnh_b0 = 1.0;
    double h_ht_tnh_b1 = 0.0;
    double h_ht_tnh_b2 = 0.0;
    double h_ht_tnh_a1 = 0.0;
    double h_ht_tnh_a2 = 0.0;
    double h_ht_tnh_z1 = 0.0;
    double h_ht_tnh_z2 = 0.0;
    double h_ht_tnl_b0 = 1.0;
    double h_ht_tnl_b1 = 0.0;
    double h_ht_tnl_b2 = 0.0;
    double h_ht_tnl_a1 = 0.0;
    double h_ht_tnl_a2 = 0.0;
    double h_ht_tnl_z1 = 0.0;
    double h_ht_tnl_z2 = 0.0;
    double h_cp_active = 0.0;
    double h_cp_trlast = 0.0;
    double h_cp_vg = 1.0;
    double h_cp_si = 0.0;
    double h_cp_maxs = 0.0;
    double h_cp_dec = 1.0;
    double h_cp_air = 0.0;
    double h_cp_nlg = 1.0;
    double h_cp_ahc = 0.0;
    double h_cp_ahi = 0.0;
    double h_cp_aho = 0.0;
    double h_cp_tapn = 192.0;
    double h_cp_pos = 0.0;
    double h_cp_bl = 0.0;
    double h_cp_la1_b0 = 1.0;
    double h_cp_la1_b1 = 0.0;
    double h_cp_la1_b2 = 0.0;
    double h_cp_la1_a1 = 0.0;
    double h_cp_la1_a2 = 0.0;
    double h_cp_la1_z1 = 0.0;
    double h_cp_la1_z2 = 0.0;
    double h_cp_la2_b0 = 1.0;
    double h_cp_la2_b1 = 0.0;
    double h_cp_la2_b2 = 0.0;
    double h_cp_la2_a1 = 0.0;
    double h_cp_la2_a2 = 0.0;
    double h_cp_la2_z1 = 0.0;
    double h_cp_la2_z2 = 0.0;
    double h_cp_lb1_b0 = 1.0;
    double h_cp_lb1_b1 = 0.0;
    double h_cp_lb1_b2 = 0.0;
    double h_cp_lb1_a1 = 0.0;
    double h_cp_lb1_a2 = 0.0;
    double h_cp_lb1_z1 = 0.0;
    double h_cp_lb1_z2 = 0.0;
    double h_cp_lb2_b0 = 1.0;
    double h_cp_lb2_b1 = 0.0;
    double h_cp_lb2_b2 = 0.0;
    double h_cp_lb2_a1 = 0.0;
    double h_cp_lb2_a2 = 0.0;
    double h_cp_lb2_z1 = 0.0;
    double h_cp_lb2_z2 = 0.0;
    double h_ch_active = 0.0;
    double h_ch_trlast = 0.0;
    double h_ch_vg = 1.0;
    double h_ch_fi = 0.0;
    double h_ch_nf = 0.0;
    double h_ch_gff = 0.0;
    double h_ch_igf = 0.0;
    double h_ch_bae = 0.0;
    double h_ch_bac = 0.999;
    double h_ch_cke = 0.0;
    double h_ch_ckc = 0.999;
    double h_ch_dcx = 0.0;
    double h_ch_dcy = 0.0;
    double h_ch_wa = 0.0;
    double h_ch_wd = 0.0;
    double h_ch_fe = 0.0;
    double h_ch_se = 0.0;
    double h_ch_fdc = 0.999;
    double h_ch_sdc = 0.999;
    double h_ch_ae = 0.0;
    double h_ch_ac = 0.01;
    double h_ch_nin = 0.0;
    double h_ch_nout = 0.0;
    double h_ch_hp_b0 = 1.0;
    double h_ch_hp_b1 = 0.0;
    double h_ch_hp_b2 = 0.0;
    double h_ch_hp_a1 = 0.0;
    double h_ch_hp_a2 = 0.0;
    double h_ch_hp_z1 = 0.0;
    double h_ch_hp_z2 = 0.0;
    double h_ch_lp_b0 = 1.0;
    double h_ch_lp_b1 = 0.0;
    double h_ch_lp_b2 = 0.0;
    double h_ch_lp_a1 = 0.0;
    double h_ch_lp_a2 = 0.0;
    double h_ch_lp_z1 = 0.0;
    double h_ch_lp_z2 = 0.0;
    double h_oh_active = 0.0;
    double h_oh_trlast = 0.0;
    double h_oh_vg = 1.0;
    double h_oh_fi = 0.0;
    double h_oh_nf = 0.0;
    double h_oh_gff = 0.0;
    double h_oh_igf = 0.0;
    double h_oh_bae = 0.0;
    double h_oh_bac = 0.999;
    double h_oh_cke = 0.0;
    double h_oh_ckc = 0.999;
    double h_oh_dcx = 0.0;
    double h_oh_dcy = 0.0;
    double h_oh_wa = 0.0;
    double h_oh_wd = 0.0;
    double h_oh_fe = 0.0;
    double h_oh_se = 0.0;
    double h_oh_fdc = 0.999;
    double h_oh_sdc = 0.999;
    double h_oh_ae = 0.0;
    double h_oh_ac = 0.01;
    double h_oh_nin = 0.0;
    double h_oh_nout = 0.0;
    double h_oh_hp_b0 = 1.0;
    double h_oh_hp_b1 = 0.0;
    double h_oh_hp_b2 = 0.0;
    double h_oh_hp_a1 = 0.0;
    double h_oh_hp_a2 = 0.0;
    double h_oh_hp_z1 = 0.0;
    double h_oh_hp_z2 = 0.0;
    double h_oh_lp_b0 = 1.0;
    double h_oh_lp_b1 = 0.0;
    double h_oh_lp_b2 = 0.0;
    double h_oh_lp_a1 = 0.0;
    double h_oh_lp_a2 = 0.0;
    double h_oh_lp_z1 = 0.0;
    double h_oh_lp_z2 = 0.0;
    double h_oh_chk = 0.0;
    double h_oh_chkg = 1.0;
    double h_cy_active = 0.0;
    double h_cy_trlast = 0.0;
    double h_cy_vg = 1.0;
    double h_cy_p0 = 0.0;
    double h_cy_p1 = 0.0;
    double h_cy_p2 = 0.0;
    double h_cy_p3 = 0.0;
    double h_cy_p4 = 0.0;
    double h_cy_p5 = 0.0;
    double h_cy_rt = 1.0;
    double h_cy_eh = 0.0;
    double h_cy_em = 0.0;
    double h_cy_ehc = 0.999;
    double h_cy_emc = 0.999;
    double h_cy_rise = 0.0;
    double h_cy_risec = 0.1;
    double h_cy_n = 0.0;
    double h_cy_gh = 1.0;
    double h_cy_gm = 1.0;
    double h_cy_dec = 0.7;
    double h_cy_h1_b0 = 1.0;
    double h_cy_h1_b1 = 0.0;
    double h_cy_h1_b2 = 0.0;
    double h_cy_h1_a1 = 0.0;
    double h_cy_h1_a2 = 0.0;
    double h_cy_h1_z1 = 0.0;
    double h_cy_h1_z2 = 0.0;
    double h_cy_h2_b0 = 1.0;
    double h_cy_h2_b1 = 0.0;
    double h_cy_h2_b2 = 0.0;
    double h_cy_h2_a1 = 0.0;
    double h_cy_h2_a2 = 0.0;
    double h_cy_h2_z1 = 0.0;
    double h_cy_h2_z2 = 0.0;
    double h_cy_m1_b0 = 1.0;
    double h_cy_m1_b1 = 0.0;
    double h_cy_m1_b2 = 0.0;
    double h_cy_m1_a1 = 0.0;
    double h_cy_m1_a2 = 0.0;
    double h_cy_m1_z1 = 0.0;
    double h_cy_m1_z2 = 0.0;
    double h_cy_m2_b0 = 1.0;
    double h_cy_m2_b1 = 0.0;
    double h_cy_m2_b2 = 0.0;
    double h_cy_m2_a1 = 0.0;
    double h_cy_m2_a2 = 0.0;
    double h_cy_m2_z1 = 0.0;
    double h_cy_m2_z2 = 0.0;
    double h_cy_hp_b0 = 1.0;
    double h_cy_hp_b1 = 0.0;
    double h_cy_hp_b2 = 0.0;
    double h_cy_hp_a1 = 0.0;
    double h_cy_hp_a2 = 0.0;
    double h_cy_hp_z1 = 0.0;
    double h_cy_hp_z2 = 0.0;

    // ---- Data / Buffer
    double ch_ph[64] = {};
    double ch_inc[64] = {};
    double ch_amp[64] = {};
    double ch_wob[64] = {};
    double oh_ph[64] = {};
    double oh_inc[64] = {};
    double oh_amp[64] = {};
    double oh_wob[64] = {};
    double cp_c[1536] = {};
    double cp_ring[1024] = {};
    double tbl[909] = {
        0.17299991247, 0.38520469294, 0.36505443856, 0.3056067922, 0.22452826026, 0.1390871492, 0.091927071889, 0.014103916149,
        -0.037942942562, -0.081746751873, -0.10526792061, -0.12205764226, -0.14334556915, -0.15716007865, -0.1784242143, -0.20677230638,
        -0.22379942677, -0.21503897132, -0.18576203109, -0.15083509179, -0.14098250958, -0.12832528888, -0.093813025596, -0.060017450617,
        -0.031084397093, -0.009470116789, -0.007790119955, 0.007178104956, 0.036756277094, 0.04505838657, 0.053116535332, 0.063101344941,
        0.071431644979, 0.078671795335, 0.068710976373, 0.054966574939, 0.05400905333, 0.057502796101, 0.070540598859, 0.073996216522,
        0.071787348948, 0.074131139235, 0.059788514933, 0.051424890013, 0.047546079593, 0.044829685002, 0.033747960232, 0.020824402508,
        0.024901738822, 0.035475715459, 0.025897916476, -0.00454277782, -0.019814509444, -0.047212428528, -0.065262464268, -0.071616739457,
        -0.098859981673, -0.10089199331, -0.072349242843, -0.053079429834, -0.052444421821, -0.05276611808, -0.055103536933, -0.043079985895,
        -0.022298520083, -0.015277499429, -0.002043266809, 0.003251879132, -0.002129042954, 0.004408275783, 0.003851059468, 0.000922883085,
        0.014605638148, 0.025834777747, 0.032312073687, 0.041136814026, 0.043009171598, 0.035981571251, 0.025953073495, 0.018527830369,
        0.01320548621, 0.007109198019, 0.010396742207, 0.020931430046, 0.031227821414, 0.04958099189, 0.054667524262, 0.045332436846,
        0.031082430763, 0.011505544386, 0.004027702903, -0.005458718557, -0.013768563198, -0.014379626752, -0.017033045254, -0.018838404111,
        -0.020550629544, -0.021246278508, -0.026315479921, -0.029134645063, -0.031006974644, -0.0311524245, -0.021434461031, -0.020322271955,
        -0.02421978967, -0.019885234655, -0.011013383269, 0.007977263019, 0.020226216757, 0.009835990353, -0.002682629989, -0.007512433292,
        -0.010426829419, -0.01724487272, -0.022549015083, -0.015310424013, -0.003772557231, -0.000684778174, -0.002798664438, -0.001700458256,
        0.006294605962, 0.009250451906, 0.006758949151, 0.013722494372, 0.019039073252, 0.022172265663, 0.028985564843, 0.030641273329,
        0.028861809496, 0.027316314986, 0.020445908089, 0.013581485055, 0.015115719828, 0.015888362331, 0.01069886731, 0.002833980286,
        -0.003235004573, -0.008577105016, -0.013926923145, -0.018129481739, -0.021631097326, -0.021335178464, -0.018788879538, -0.01465348179,
        -0.008935704875, -0.006225323112, -0.007243639431, -0.009222228434, -0.009926752048, -0.010635525159, -0.010652959172, -0.00824897912,
        -0.009592642701, -0.009787500538, -0.003860744206, 0.000383599768, 0.004092494974, 0.004249431582, 0.002512795841, 0.006743432616,
        0.008947574517, 0.006792703982, 0.00449368976, 0.002525486238, 0.00362293088, 0.007001171652, 0.008731944303, 0.008943122532,
        0.008552978904, 0.007276823619, 0.006126337279, 0.005264636028, 0.003986823047, 0.003074942605, 0.002988281075, 0.002460360681,
        0.001073914903, -0.000195899001, -0.001433660896, -0.002291697432, -0.00222586385, -0.001823423495, -0.001283969238, -0.000935763728,
        -0.000794971817, -0.000610054994, -0.000406694339, -0.000218519355, -0.000088834898, -0.000025824788, -0.000004945448, 0.0,
        0.18875619402, 0.39532535099, 0.31052115012, 0.20417681294, 0.12327780667, 0.053936258901, 0.022253809872, -0.046593291093,
        -0.10347847143, -0.16319263581, -0.18598616783, -0.19691378438, -0.19998351978, -0.19882124185, -0.19398388684, -0.17466283449,
        -0.17442871397, -0.16680812656, -0.14932833745, -0.11161678039, -0.079639914597, -0.06049480322, -0.043097509189, -0.037946681895,
        -0.027856113623, -0.004052847128, 0.024570897462, 0.051935184672, 0.073481992541, 0.080365700782, 0.093774064874, 0.11429755485,
        0.12814304655, 0.13704431024, 0.13654880269, 0.1338437707, 0.12377069562, 0.11334862119, 0.11755749462, 0.11280667005,
        0.088223363515, 0.060171909313, 0.043882991093, 0.031759968057, 0.010557516928, 0.00634073903, 0.022863515741, 0.017511511564,
        -0.011691636252, -0.040678974631, -0.05941505959, -0.066718975025, -0.066513089303, -0.059587464418, -0.061848135339, -0.070400122558,
        -0.07495297763, -0.080457044658, -0.07764401586, -0.06118855765, -0.041233682302, -0.033080606371, -0.035374527778, -0.041106987629,
        -0.040476444817, -0.036943129798, -0.037512208948, -0.039119038538, -0.036975340594, -0.014263153464, 0.008080013507, 0.017602733217,
        0.022936607116, 0.022241988585, 0.024327434638, 0.035654667183, 0.051179527021, 0.050305826316, 0.039597608314, 0.031995287216,
        0.028936693434, 0.029734677245, 0.026427212406, 0.027868867792, 0.030683092212, 0.028941365711, 0.025365597524, 0.016343393416,
        0.00886177259, 0.004053934594, 0.000323165951, -0.000552734124, 0.000301399868, 0.000644844542, -0.002598426848, -0.007375652056,
        -0.017241810261, -0.016335407228, -0.00728530571, -0.01027161321, -0.008202411, -0.005886343365, -0.010774098558, -0.011499064632,
        -0.001034034188, 0.019238286584, 0.02419221092, 0.01072712983, 0.002477773332, 0.001021558224, 0.002455105118, 0.009388991691,
        0.004028217825, -0.00875615369, -0.004909269878, -0.000861395665, -0.009547048026, -0.021571357383, -0.028191072376, -0.027298650306,
        -0.024646025014, -0.022188370626, -0.022663653592, -0.0169210777, -0.005878314095, -0.003069215548, -0.002581063403, -0.003054751621,
        -0.002067677169, -0.002296400571, -0.007026141858, -0.008682390266, -0.01369061601, -0.015594866246, -0.015848685375, -0.014647070009,
        0.000214771541, 0.017655075723, 0.030585570046, 0.035469789746, 0.033094484497, 0.026992796577, 0.026424372263, 0.031620824835,
        0.031452315987, 0.024288905438, 0.021992004564, 0.029756968347, 0.026089126997, 0.021342070492, 0.022254626751, 0.019769410909,
        0.016155106566, 0.004306405035, -0.006203864236, -0.010095456038, -0.013423823436, -0.019278293978, -0.019673820252, -0.011910960372,
        -0.013594786202, -0.021142244028, -0.027204330424, -0.03106817663, -0.0278949022, -0.027153380416, -0.028041008611, -0.024219212736,
        -0.020634875707, -0.016187012168, -0.010479424511, -0.007266352768, -0.005637441673, -0.003742269821, -0.001416368025, 0.000339280046,
        0.002342001188, 0.004716215272, 0.005427957512, 0.005564537635, 0.005551386147, 0.004393799631, 0.003251158122, 0.002978309206,
        0.002795176985, 0.00224792998, 0.001399587713, 0.000699048645, 0.000367048904, 0.000177139081, 0.000037858337, 0.0,
        0.14644279033, 0.33251425492, 0.30859052842, 0.23850248802, 0.17106916088, 0.088562854779, 0.031893734688, -0.030031178833,
        -0.079945965465, -0.13244572685, -0.16362719173, -0.17710008278, -0.19040712613, -0.20589490809, -0.21188769343, -0.20689380767,
        -0.20043829364, -0.18881694375, -0.17639896033, -0.15042825626, -0.11744597235, -0.078919664332, -0.044104005328, -0.003890454167,
        0.030533000837, 0.044945801627, 0.065836836767, 0.082453527556, 0.092875174988, 0.097217019887, 0.10303531878, 0.10783131664,
        0.10028010263, 0.099973070631, 0.09570897306, 0.083961725978, 0.080039703641, 0.088430083766, 0.095913529808, 0.087543081064,
        0.079121999752, 0.075578302503, 0.071182878368, 0.057252560998, 0.039309422778, 0.02517176428, 0.004465357332, -0.011864384777,
        -0.030366262986, -0.055890672791, -0.075085014848, -0.086867688815, -0.088778224349, -0.090584711493, -0.092215571744, -0.091266542927,
        -0.092624401986, -0.096245312917, -0.08964877015, -0.069226921724, -0.055988772723, -0.044530985568, -0.026720085666, -0.007970979895,
        0.00912585792, 0.02330521671, 0.032412006527, 0.033089622507, 0.035608404131, 0.039076460601, 0.034604297928, 0.033155923189,
        0.041606745316, 0.04639651332, 0.048867472567, 0.056108334898, 0.060791044239, 0.061414992382, 0.053634383802, 0.040484233121,
        0.025334577443, 0.006959282872, -0.005507223119, -0.01358040347, -0.024465813753, -0.030377069583, -0.025157539555, -0.022014305929,
        -0.018436730819, -0.013259652274, -0.015716725316, -0.015201767541, -0.010464498135, -0.010328027209, -0.013266340203, -0.009330777397,
        -0.003369878001, 0.002233106847, 0.008353461548, 0.011517625483, 0.011225234035, 0.005661226426, 0.00846871158, 0.011962385205,
        0.005675051508, -0.003419607922, -0.011423524889, -0.015239401905, -0.016309600843, -0.017943625745, -0.018068378795, -0.008890300415,
        -0.003573186458, -0.005592307119, -0.007961568244, -0.011019803958, -0.008374190712, -0.006080136134, -0.005935820305, -0.004467513715,
        -0.00550963031, -0.000779047237, 0.011686518541, 0.015498594718, 0.011043152991, 0.006869424843, 0.003978515041, 0.005637761255,
        0.007684487239, 0.009047564734, 0.013301744585, 0.018719059063, 0.023454687156, 0.022704440167, 0.021347777532, 0.021712409763,
        0.01724840343, 0.011262126584, 0.008482470421, 0.006044060596, -0.001671022845, -0.004404271042, -0.001806840909, -0.00375505995,
        -0.005367355603, -0.006384821254, -0.005363243861, -0.004776021728, -0.008734727083, -0.0129699792, -0.014830935442, -0.013895969576,
        -0.015001843702, -0.017324794169, -0.017296900826, -0.01813991501, -0.021433352777, -0.026018132861, -0.02609554886, -0.019520000378,
        -0.011810525169, -0.006403612026, -0.002075881141, 0.001473547588, 0.004369448479, 0.009145887713, 0.011001737153, 0.009167555085,
        0.008188232622, 0.008367470454, 0.00949509977, 0.009499334377, 0.009389855878, 0.009090164865, 0.007214555676, 0.005284695326,
        0.003824214994, 0.002276197524, 0.000981108364, -0.000207805217, -0.001530639148, -0.001684870417, -0.000884046753, -0.000307764052,
        -0.000227864591, -0.000124613494, 0.000054600497, 0.000055594324, 0.000007264971, -0.000009333485, -0.000003709795, 0.0,
        0.087130201318, 0.22155809298, 0.28348361607, 0.28969521695, 0.25567232669, 0.20070052321, 0.13676317259, 0.074221264546,
        0.021740344735, -0.03223555278, -0.079975832576, -0.12418344686, -0.15733516311, -0.18224817432, -0.20354665656, -0.21595437326,
        -0.2184011454, -0.21408287478, -0.20775063975, -0.19588524792, -0.17977876415, -0.15828956198, -0.13631974334, -0.11430055582,
        -0.089797315789, -0.062661166421, -0.035405955397, -0.013159330961, 0.009600020323, 0.031611400021, 0.051726064218, 0.068934579219,
        0.082731548195, 0.093974995507, 0.10207550769, 0.11138532055, 0.11672223674, 0.1180063842, 0.1193989218, 0.11432582188,
        0.10506392496, 0.096589559732, 0.088206046829, 0.079076695511, 0.066358888242, 0.052976560198, 0.040920832018, 0.027033126345,
        0.012788449284, -0.000835223893, -0.014258011163, -0.027707175256, -0.040898501891, -0.051405484331, -0.057644255084, -0.061241170245,
        -0.064352140841, -0.066777290497, -0.068791522233, -0.068268254727, -0.062136623545, -0.04984442561, -0.036728842541, -0.027176030821,
        -0.019985779258, -0.016729785576, -0.010199577848, 0.001196362223, 0.010580382277, 0.017103609343, 0.020289546288, 0.022227453956,
        0.023310296858, 0.025875142251, 0.03014834754, 0.032651095324, 0.034763920863, 0.035975117165, 0.034584726143, 0.031687552408,
        0.028861034451, 0.023490087772, 0.016815260403, 0.010886991352, 0.00515740912, 0.001078737294, -0.004208831383, -0.009110507197,
        -0.012059160842, -0.014331597218, -0.014107396181, -0.014870141096, -0.018241726463, -0.021682008998, -0.024036347026, -0.023494730691,
        -0.020688706205, -0.01832688981, -0.015230415289, -0.011777225703, -0.007152438673, -0.001410194325, 0.001541874136, 0.003645928511,
        0.006247635228, 0.007722346447, 0.008333769517, 0.009459755919, 0.010568134465, 0.009220300056, 0.007724042766, 0.008740436712,
        0.009038912872, 0.007568168482, 0.0063264442, 0.005460072617, 0.003820831196, 0.002875813451, 0.004200546154, 0.005406205386,
        0.006444472754, 0.006831180817, 0.005420008129, 0.003667325472, 0.000941381773, -0.001277675118, -0.001378376857, 0.000252547766,
        0.002414187717, 0.002778782603, 0.000811258464, -0.002276271177, -0.0042076427, -0.005359203194, -0.007029061555, -0.007584708039,
        -0.006605300072, -0.005248875539, -0.005368223991, -0.005994690846, -0.006042700377, -0.007176808945, -0.007126869349, -0.004615879152,
        -0.002679401685, -0.001316804511, 0.001016261771, 0.002765049979, 0.003768256683, 0.004481381732, 0.00379701857, 0.001935285991,
        0.000288377916, 0.00059387704, 0.001712379452, 0.002343497728, 0.002552321951, 0.001392233743, -0.001084704455, -0.003299724802,
        -0.003423601794, -0.001179544511, 0.001051597148, 0.000933066263, 0.000848237368, 0.001473863331, 0.00172784357, 0.00195002283,
        0.001909872303, 0.001983412128, 0.002188697856, 0.00227517267, 0.002327850003, 0.002350595298, 0.001792637079, 0.000957874239,
        0.00042225305, 0.000251864896, 0.000091255406, -0.00039853993, -0.000777675601, -0.000876740231, -0.000780060748, -0.000578594959,
        -0.000414276151, -0.000273007016, -0.000171151478, -0.000092326999, -0.000043627118, -0.000020987719, -0.000005385739, 0.0,
        3804.0, 4270.0, 4500.0, 4632.0, 4800.0, 4900.0, 5140.0, 5400.0,
        5500.0, 5610.0, 5978.0, 6340.0, 6500.0, 6600.0, 6701.0, 7106.0,
        7200.0, 7339.0, 7500.0, 7686.0, 7854.0, 7938.0, 8048.0, 8399.0,
        8514.0, 8601.0, 8700.0, 8876.0, 9000.0, 9100.0, 9394.0, 9500.0,
        9600.0, 9756.0, 9899.0, 10143.0, 10300.0, 10800.0, 11201.0, 11571.0,
        11673.0, 12318.0, 12680.0, 12841.0, 13340.0, 13663.0, 16122.0, 0.072,
        0.077, 0.052, 0.038, 0.033, 0.099, 0.049, 0.116, 0.116,
        0.088, 0.742, 1.3, 0.181, 0.178, 0.05, 0.126, 0.128,
        0.04, 0.132, 0.573, 0.086, 0.039, 0.062, 0.078, 0.081,
        0.05, 0.045, 0.166, 0.051, 0.081, 0.101, 0.052, 0.064,
        0.045, 0.042, 0.062, 0.034, 0.043, 0.037, 0.036, 0.041,
        0.06, 0.09, 0.033, 0.045, 0.034, 0.03, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0, 0.0,
    };

    double fs_ = 44100.0;
    double vout[8] = {};   // per-drum signals of the last tick (before the master volume)
    uint32_t rng_ = 0x12345678u;

    inline double noise_() {   // white noise in [-1, 1)
        rng_ ^= rng_ << 13; rng_ ^= rng_ >> 17; rng_ ^= rng_ << 5;
        return (double) rng_ * (2.0 / 4294967296.0) - 1.0;
    }
    template <std::size_t N> static inline double peek_(const double (&d)[N], double idx) {
        const int i = (int) std::floor(idx);
        return (i >= 0 && i < (int) N) ? d[(std::size_t) i] : 0.0;
    }
    template <std::size_t N> static inline void poke_(double (&d)[N], double v, double idx) {
        const int i = (int) std::floor(idx);
        if (i >= 0 && i < (int) N) d[(std::size_t) i] = v;
    }

    // one sample; returns the mono mix
    inline double tick() {
        double acc = 0.0, anysolo = 0.0, bd_D = 0.0, bd_T = 0.0, bd_active = 0.0, bd_amp = 0.0;
        double bd_ampc = 0.0, bd_amt = 0.0, bd_bla = 0.0, bd_blz = 0.0, bd_cc = 0.0, bd_ce = 0.0;
        double bd_cla = 0.0, bd_clz = 0.0, bd_dcx = 0.0, bd_dcy = 0.0, bd_dec = 0.0, bd_dl = 0.0;
        double bd_end = 0.0, bd_fire = 0.0, bd_hp_a1 = 0.0, bd_hp_a2 = 0.0, bd_hp_b0 = 0.0, bd_hp_b1 = 0.0;
        double bd_hp_b2 = 0.0, bd_hp_t1al = 0.0, bd_hp_t1c = 0.0, bd_hp_t1cw = 0.0, bd_hp_t1ia = 0.0, bd_hp_t1q = 0.0;
        double bd_hp_t1sw = 0.0, bd_hp_t1w = 0.0, bd_hp_z1 = 0.0, bd_hp_z2 = 0.0, bd_hpo = 0.0, bd_hz = 0.0;
        double bd_ic = 0.0, bd_ie = 0.0, bd_ila = 0.0, bd_ilz = 0.0, bd_last = 0.0, bd_mg = 0.0;
        double bd_mt = 0.0, bd_n = 0.0, bd_nin = 0.0, bd_nout = 0.0, bd_nx = 0.0, bd_out = 0.0;
        double bd_pc = 0.0, bd_pd = 0.0, bd_ph = 0.0, bd_ratio = 0.0, bd_raw = 0.0, bd_sh = 0.0;
        double bd_sil = 0.0, bd_sq = 0.0, bd_start = 0.0, bd_sum = 0.0, bd_tg = 0.0, bd_thud = 0.0;
        double bd_trlast = 0.0, bd_ts = 0.0, bd_vel = 0.0, bd_vg = 0.0, bd_y = 0.0, ch_ac = 0.0;
        double ch_active = 0.0, ch_ae = 0.0, ch_bac = 0.0, ch_bae = 0.0, ch_bd = 0.0, ch_bg = 0.0;
        double ch_ckc = 0.0, ch_cke = 0.0, ch_dcx = 0.0, ch_dcy = 0.0, ch_dec = 0.0, ch_env = 0.0;
        double ch_f = 0.0, ch_fdc = 0.0, ch_fe = 0.0, ch_fi = 0.0, ch_fire = 0.0, ch_fr = 0.0;
        double ch_gff = 0.0, ch_hp_a1 = 0.0, ch_hp_a2 = 0.0, ch_hp_b0 = 0.0, ch_hp_b1 = 0.0, ch_hp_b2 = 0.0;
        double ch_hp_t22al = 0.0, ch_hp_t22c = 0.0, ch_hp_t22cw = 0.0, ch_hp_t22ia = 0.0, ch_hp_t22q = 0.0, ch_hp_t22sw = 0.0;
        double ch_hp_t22w = 0.0, ch_hp_z1 = 0.0, ch_hp_z2 = 0.0, ch_ic = 0.0, ch_igf = 0.0, ch_left = 0.0;
        double ch_lg = 0.0, ch_lp_a1 = 0.0, ch_lp_a2 = 0.0, ch_lp_b0 = 0.0, ch_lp_b1 = 0.0, ch_lp_b2 = 0.0;
        double ch_lp_t23al = 0.0, ch_lp_t23c = 0.0, ch_lp_t23cw = 0.0, ch_lp_t23ia = 0.0, ch_lp_t23q = 0.0, ch_lp_t23sw = 0.0;
        double ch_lp_t23w = 0.0, ch_lp_z1 = 0.0, ch_lp_z2 = 0.0, ch_lv = 0.0, ch_mg = 0.0, ch_mt = 0.0;
        double ch_n1 = 0.0, ch_nd = 0.0, ch_nf = 0.0, ch_nin = 0.0, ch_nout = 0.0, ch_nx = 0.0;
        double ch_nz = 0.0, ch_o = 0.0, ch_out = 0.0, ch_pv = 0.0, ch_rr = 0.0, ch_sdc = 0.0;
        double ch_se = 0.0, ch_sq = 0.0, ch_src = 0.0, ch_ton = 0.0, ch_tp = 0.0, ch_trlast = 0.0;
        double ch_vel = 0.0, ch_vg = 0.0, ch_wa = 0.0, ch_wd = 0.0, ch_wn = 0.0, ch_wv = 0.0;
        double ch_y = 0.0, cp_active = 0.0, cp_ahc = 0.0, cp_ahi = 0.0, cp_aho = 0.0, cp_air = 0.0;
        double cp_am = 0.0, cp_base = 0.0, cp_bl = 0.0, cp_blf = 0.0, cp_c0 = 0.0, cp_c1 = 0.0;
        double cp_c2 = 0.0, cp_c3 = 0.0, cp_cal = 0.0, cp_ci = 0.0, cp_cn = 0.0, cp_cv = 0.0;
        double cp_da = 0.0, cp_db2 = 0.0, cp_dds = 0.0, cp_dec = 0.0, cp_dg = 0.0, cp_dm = 0.0;
        double cp_dn = 0.0, cp_dr = 0.0, cp_dr2 = 0.0, cp_dry = 0.0, cp_f2 = 0.0, cp_f3 = 0.0;
        double cp_fa = 0.0, cp_fade = 0.0, cp_fb = 0.0, cp_fb2 = 0.0, cp_fire = 0.0, cp_fm = 0.0;
        double cp_fp = 0.0, cp_fr = 0.0, cp_full = 0.0, cp_g = 0.0, cp_ii = 0.0, cp_la1_a1 = 0.0;
        double cp_la1_a2 = 0.0, cp_la1_b0 = 0.0, cp_la1_b1 = 0.0, cp_la1_b2 = 0.0, cp_la1_t18al = 0.0, cp_la1_t18c = 0.0;
        double cp_la1_t18cw = 0.0, cp_la1_t18ia = 0.0, cp_la1_t18q = 0.0, cp_la1_t18sw = 0.0, cp_la1_t18w = 0.0, cp_la1_z1 = 0.0;
        double cp_la1_z2 = 0.0, cp_la2_a1 = 0.0, cp_la2_a2 = 0.0, cp_la2_b0 = 0.0, cp_la2_b1 = 0.0, cp_la2_b2 = 0.0;
        double cp_la2_t19al = 0.0, cp_la2_t19c = 0.0, cp_la2_t19cw = 0.0, cp_la2_t19ia = 0.0, cp_la2_t19q = 0.0, cp_la2_t19sw = 0.0;
        double cp_la2_t19w = 0.0, cp_la2_z1 = 0.0, cp_la2_z2 = 0.0, cp_lb1_a1 = 0.0, cp_lb1_a2 = 0.0, cp_lb1_b0 = 0.0;
        double cp_lb1_b1 = 0.0, cp_lb1_b2 = 0.0, cp_lb1_t20al = 0.0, cp_lb1_t20c = 0.0, cp_lb1_t20cw = 0.0, cp_lb1_t20ia = 0.0;
        double cp_lb1_t20q = 0.0, cp_lb1_t20sw = 0.0, cp_lb1_t20w = 0.0, cp_lb1_z1 = 0.0, cp_lb1_z2 = 0.0, cp_lb2_a1 = 0.0;
        double cp_lb2_a2 = 0.0, cp_lb2_b0 = 0.0, cp_lb2_b1 = 0.0, cp_lb2_b2 = 0.0, cp_lb2_t21al = 0.0, cp_lb2_t21c = 0.0;
        double cp_lb2_t21cw = 0.0, cp_lb2_t21ia = 0.0, cp_lb2_t21q = 0.0, cp_lb2_t21sw = 0.0, cp_lb2_t21w = 0.0, cp_lb2_z1 = 0.0;
        double cp_lb2_z2 = 0.0, cp_lm = 0.0, cp_lw = 0.0, cp_maxs = 0.0, cp_mg = 0.0, cp_mt = 0.0;
        double cp_nlg = 0.0, cp_nn = 0.0, cp_out = 0.0, cp_p0 = 0.0, cp_p1 = 0.0, cp_p2 = 0.0;
        double cp_p3 = 0.0, cp_pos = 0.0, cp_ps = 0.0, cp_re = 0.0, cp_rt = 0.0, cp_s = 0.0;
        double cp_sat = 0.0, cp_sc = 0.0, cp_se = 0.0, cp_short = 0.0, cp_si = 0.0, cp_sn = 0.0;
        double cp_snp = 0.0, cp_sq = 0.0, cp_stp = 0.0, cp_sv = 0.0, cp_ta = 0.0, cp_tapn = 0.0;
        double cp_tat = 0.0, cp_tc = 0.0, cp_tda = 0.0, cp_tenv = 0.0, cp_tm = 0.0, cp_trlast = 0.0;
        double cp_vel = 0.0, cp_vg = 0.0, cy_a1 = 0.0, cy_a2 = 0.0, cy_active = 0.0, cy_b1 = 0.0;
        double cy_b2 = 0.0, cy_dec = 0.0, cy_dt0 = 0.0, cy_dt1 = 0.0, cy_dt2 = 0.0, cy_dt3 = 0.0;
        double cy_dt4 = 0.0, cy_dt5 = 0.0, cy_eh = 0.0, cy_ehc = 0.0, cy_em = 0.0, cy_emc = 0.0;
        double cy_fire = 0.0, cy_gh = 0.0, cy_gm = 0.0, cy_h1_a1 = 0.0, cy_h1_a2 = 0.0, cy_h1_b0 = 0.0;
        double cy_h1_b1 = 0.0, cy_h1_b2 = 0.0, cy_h1_t26al = 0.0, cy_h1_t26c = 0.0, cy_h1_t26cw = 0.0, cy_h1_t26ia = 0.0;
        double cy_h1_t26q = 0.0, cy_h1_t26sw = 0.0, cy_h1_t26w = 0.0, cy_h1_z1 = 0.0, cy_h1_z2 = 0.0, cy_h2_a1 = 0.0;
        double cy_h2_a2 = 0.0, cy_h2_b0 = 0.0, cy_h2_b1 = 0.0, cy_h2_b2 = 0.0, cy_h2_t27al = 0.0, cy_h2_t27c = 0.0;
        double cy_h2_t27cw = 0.0, cy_h2_t27ia = 0.0, cy_h2_t27q = 0.0, cy_h2_t27sw = 0.0, cy_h2_t27w = 0.0, cy_h2_z1 = 0.0;
        double cy_h2_z2 = 0.0, cy_hf0 = 0.0, cy_hf1 = 0.0, cy_hf2 = 0.0, cy_hf3 = 0.0, cy_hf4 = 0.0;
        double cy_hf5 = 0.0, cy_hp_a1 = 0.0, cy_hp_a2 = 0.0, cy_hp_b0 = 0.0, cy_hp_b1 = 0.0, cy_hp_b2 = 0.0;
        double cy_hp_t30al = 0.0, cy_hp_t30c = 0.0, cy_hp_t30cw = 0.0, cy_hp_t30ia = 0.0, cy_hp_t30q = 0.0, cy_hp_t30sw = 0.0;
        double cy_hp_t30w = 0.0, cy_hp_z1 = 0.0, cy_hp_z2 = 0.0, cy_hpo = 0.0, cy_m1_a1 = 0.0, cy_m1_a2 = 0.0;
        double cy_m1_b0 = 0.0, cy_m1_b1 = 0.0, cy_m1_b2 = 0.0, cy_m1_t28al = 0.0, cy_m1_t28c = 0.0, cy_m1_t28cw = 0.0;
        double cy_m1_t28ia = 0.0, cy_m1_t28q = 0.0, cy_m1_t28sw = 0.0, cy_m1_t28w = 0.0, cy_m1_z1 = 0.0, cy_m1_z2 = 0.0;
        double cy_m2_a1 = 0.0, cy_m2_a2 = 0.0, cy_m2_b0 = 0.0, cy_m2_b1 = 0.0, cy_m2_b2 = 0.0, cy_m2_t29al = 0.0;
        double cy_m2_t29c = 0.0, cy_m2_t29cw = 0.0, cy_m2_t29ia = 0.0, cy_m2_t29q = 0.0, cy_m2_t29sw = 0.0, cy_m2_t29w = 0.0;
        double cy_m2_z1 = 0.0, cy_m2_z2 = 0.0, cy_mg = 0.0, cy_mix = 0.0, cy_mt = 0.0, cy_n = 0.0;
        double cy_nv0 = 0.0, cy_nv1 = 0.0, cy_nv2 = 0.0, cy_nv3 = 0.0, cy_nv4 = 0.0, cy_nv5 = 0.0;
        double cy_out = 0.0, cy_p0 = 0.0, cy_p1 = 0.0, cy_p2 = 0.0, cy_p3 = 0.0, cy_p4 = 0.0;
        double cy_p5 = 0.0, cy_rise = 0.0, cy_risec = 0.0, cy_rt = 0.0, cy_sq = 0.0, cy_tm = 0.0;
        double cy_tn = 0.0, cy_tr = 0.0, cy_trlast = 0.0, cy_vel = 0.0, cy_vg = 0.0, fs = 0.0;
        double ht_active = 0.0, ht_br = 0.0, ht_brc = 0.0, ht_bue = 0.0, ht_bup = 0.0, ht_burst = 0.0;
        double ht_dec = 0.0, ht_exc = 0.0, ht_fire = 0.0, ht_glide = 0.0, ht_gpole = 0.0, ht_hn1 = 0.0;
        double ht_hn2 = 0.0, ht_hnh_a1 = 0.0, ht_hnh_a2 = 0.0, ht_hnh_b0 = 0.0, ht_hnh_b1 = 0.0, ht_hnh_b2 = 0.0;
        double ht_hnh_t14al = 0.0, ht_hnh_t14c = 0.0, ht_hnh_t14cw = 0.0, ht_hnh_t14ia = 0.0, ht_hnh_t14q = 0.0, ht_hnh_t14sw = 0.0;
        double ht_hnh_t14w = 0.0, ht_hnh_z1 = 0.0, ht_hnh_z2 = 0.0, ht_hnl_a1 = 0.0, ht_hnl_a2 = 0.0, ht_hnl_b0 = 0.0;
        double ht_hnl_b1 = 0.0, ht_hnl_b2 = 0.0, ht_hnl_t15al = 0.0, ht_hnl_t15c = 0.0, ht_hnl_t15cw = 0.0, ht_hnl_t15ia = 0.0;
        double ht_hnl_t15q = 0.0, ht_hnl_t15sw = 0.0, ht_hnl_t15w = 0.0, ht_hnl_z1 = 0.0, ht_hnl_z2 = 0.0, ht_hs = 0.0;
        double ht_le = 0.0, ht_lhz = 0.0, ht_ln = 0.0, ht_ln1 = 0.0, ht_ln2 = 0.0, ht_lnh_a1 = 0.0;
        double ht_lnh_a2 = 0.0, ht_lnh_b0 = 0.0, ht_lnh_b1 = 0.0, ht_lnh_b2 = 0.0, ht_lnh_t12al = 0.0, ht_lnh_t12c = 0.0;
        double ht_lnh_t12cw = 0.0, ht_lnh_t12ia = 0.0, ht_lnh_t12q = 0.0, ht_lnh_t12sw = 0.0, ht_lnh_t12w = 0.0, ht_lnh_z1 = 0.0;
        double ht_lnh_z2 = 0.0, ht_lnl_a1 = 0.0, ht_lnl_a2 = 0.0, ht_lnl_b0 = 0.0, ht_lnl_b1 = 0.0, ht_lnl_b2 = 0.0;
        double ht_lnl_t13al = 0.0, ht_lnl_t13c = 0.0, ht_lnl_t13cw = 0.0, ht_lnl_t13ia = 0.0, ht_lnl_t13q = 0.0, ht_lnl_t13sw = 0.0;
        double ht_lnl_t13w = 0.0, ht_lnl_z1 = 0.0, ht_lnl_z2 = 0.0, ht_lower = 0.0, ht_lph = 0.0, ht_lpole = 0.0;
        double ht_main = 0.0, ht_me = 0.0, ht_mg = 0.0, ht_mhz = 0.0, ht_mph = 0.0, ht_mpole = 0.0;
        double ht_mt = 0.0, ht_nr = 0.0, ht_nrc = 0.0, ht_out = 0.0, ht_rt = 0.0, ht_se = 0.0;
        double ht_shz = 0.0, ht_snap = 0.0, ht_sne = 0.0, ht_snp = 0.0, ht_snr = 0.0, ht_snrc = 0.0;
        double ht_sph = 0.0, ht_spole = 0.0, ht_sq = 0.0, ht_sr = 0.0, ht_src = 0.0, ht_strike = 0.0;
        double ht_tn = 0.0, ht_tn1 = 0.0, ht_tn2 = 0.0, ht_tne = 0.0, ht_tnh_a1 = 0.0, ht_tnh_a2 = 0.0;
        double ht_tnh_b0 = 0.0, ht_tnh_b1 = 0.0, ht_tnh_b2 = 0.0, ht_tnh_t16al = 0.0, ht_tnh_t16c = 0.0, ht_tnh_t16cw = 0.0;
        double ht_tnh_t16ia = 0.0, ht_tnh_t16q = 0.0, ht_tnh_t16sw = 0.0, ht_tnh_t16w = 0.0, ht_tnh_z1 = 0.0, ht_tnh_z2 = 0.0;
        double ht_tnl_a1 = 0.0, ht_tnl_a2 = 0.0, ht_tnl_b0 = 0.0, ht_tnl_b1 = 0.0, ht_tnl_b2 = 0.0, ht_tnl_t17al = 0.0;
        double ht_tnl_t17c = 0.0, ht_tnl_t17cw = 0.0, ht_tnl_t17ia = 0.0, ht_tnl_t17q = 0.0, ht_tnl_t17sw = 0.0, ht_tnl_t17w = 0.0;
        double ht_tnl_z1 = 0.0, ht_tnl_z2 = 0.0, ht_tnp = 0.0, ht_trlast = 0.0, ht_ue0 = 0.0, ht_ue1 = 0.0;
        double ht_ue2 = 0.0, ht_ue3 = 0.0, ht_uhz0 = 0.0, ht_uhz1 = 0.0, ht_uhz2 = 0.0, ht_uhz3 = 0.0;
        double ht_up0 = 0.0, ht_up1 = 0.0, ht_up2 = 0.0, ht_up3 = 0.0, ht_uph0 = 0.0, ht_uph1 = 0.0;
        double ht_uph2 = 0.0, ht_uph3 = 0.0, ht_upper = 0.0, ht_vel = 0.0, ht_vg = 0.0, i = 0.0;
        double j = 0.0, k = 0.0, lt_active = 0.0, lt_br = 0.0, lt_brc = 0.0, lt_bue = 0.0;
        double lt_bup = 0.0, lt_burst = 0.0, lt_dec = 0.0, lt_exc = 0.0, lt_fire = 0.0, lt_fs1 = 0.0;
        double lt_fs2 = 0.0, lt_fsh_a1 = 0.0, lt_fsh_a2 = 0.0, lt_fsh_b0 = 0.0, lt_fsh_b1 = 0.0, lt_fsh_b2 = 0.0;
        double lt_fsh_t8al = 0.0, lt_fsh_t8c = 0.0, lt_fsh_t8cw = 0.0, lt_fsh_t8ia = 0.0, lt_fsh_t8q = 0.0, lt_fsh_t8sw = 0.0;
        double lt_fsh_t8w = 0.0, lt_fsh_z1 = 0.0, lt_fsh_z2 = 0.0, lt_fsl_a1 = 0.0, lt_fsl_a2 = 0.0, lt_fsl_b0 = 0.0;
        double lt_fsl_b1 = 0.0, lt_fsl_b2 = 0.0, lt_fsl_t9al = 0.0, lt_fsl_t9c = 0.0, lt_fsl_t9cw = 0.0, lt_fsl_t9ia = 0.0;
        double lt_fsl_t9q = 0.0, lt_fsl_t9sw = 0.0, lt_fsl_t9w = 0.0, lt_fsl_z1 = 0.0, lt_fsl_z2 = 0.0, lt_glide = 0.0;
        double lt_gpole = 0.0, lt_hn1 = 0.0, lt_hn2 = 0.0, lt_hnh_a1 = 0.0, lt_hnh_a2 = 0.0, lt_hnh_b0 = 0.0;
        double lt_hnh_b1 = 0.0, lt_hnh_b2 = 0.0, lt_hnh_t6al = 0.0, lt_hnh_t6c = 0.0, lt_hnh_t6cw = 0.0, lt_hnh_t6ia = 0.0;
        double lt_hnh_t6q = 0.0, lt_hnh_t6sw = 0.0, lt_hnh_t6w = 0.0, lt_hnh_z1 = 0.0, lt_hnh_z2 = 0.0, lt_hnl_a1 = 0.0;
        double lt_hnl_a2 = 0.0, lt_hnl_b0 = 0.0, lt_hnl_b1 = 0.0, lt_hnl_b2 = 0.0, lt_hnl_t7al = 0.0, lt_hnl_t7c = 0.0;
        double lt_hnl_t7cw = 0.0, lt_hnl_t7ia = 0.0, lt_hnl_t7q = 0.0, lt_hnl_t7sw = 0.0, lt_hnl_t7w = 0.0, lt_hnl_z1 = 0.0;
        double lt_hnl_z2 = 0.0, lt_hs = 0.0, lt_le = 0.0, lt_lhz = 0.0, lt_ln = 0.0, lt_ln1 = 0.0;
        double lt_ln2 = 0.0, lt_lnh_a1 = 0.0, lt_lnh_a2 = 0.0, lt_lnh_b0 = 0.0, lt_lnh_b1 = 0.0, lt_lnh_b2 = 0.0;
        double lt_lnh_t4al = 0.0, lt_lnh_t4c = 0.0, lt_lnh_t4cw = 0.0, lt_lnh_t4ia = 0.0, lt_lnh_t4q = 0.0, lt_lnh_t4sw = 0.0;
        double lt_lnh_t4w = 0.0, lt_lnh_z1 = 0.0, lt_lnh_z2 = 0.0, lt_lnl_a1 = 0.0, lt_lnl_a2 = 0.0, lt_lnl_b0 = 0.0;
        double lt_lnl_b1 = 0.0, lt_lnl_b2 = 0.0, lt_lnl_t5al = 0.0, lt_lnl_t5c = 0.0, lt_lnl_t5cw = 0.0, lt_lnl_t5ia = 0.0;
        double lt_lnl_t5q = 0.0, lt_lnl_t5sw = 0.0, lt_lnl_t5w = 0.0, lt_lnl_z1 = 0.0, lt_lnl_z2 = 0.0, lt_lower = 0.0;
        double lt_lph = 0.0, lt_lpole = 0.0, lt_main = 0.0, lt_me = 0.0, lt_mg = 0.0, lt_mhz = 0.0;
        double lt_mph = 0.0, lt_mpole = 0.0, lt_mt = 0.0, lt_nr = 0.0, lt_nrc = 0.0, lt_out = 0.0;
        double lt_rt = 0.0, lt_se = 0.0, lt_shz = 0.0, lt_snap = 0.0, lt_sne = 0.0, lt_snp = 0.0;
        double lt_snr = 0.0, lt_snrc = 0.0, lt_sph = 0.0, lt_spole = 0.0, lt_sq = 0.0, lt_sr = 0.0;
        double lt_src = 0.0, lt_strike = 0.0, lt_tn = 0.0, lt_tn1 = 0.0, lt_tn2 = 0.0, lt_tne = 0.0;
        double lt_tnh_a1 = 0.0, lt_tnh_a2 = 0.0, lt_tnh_b0 = 0.0, lt_tnh_b1 = 0.0, lt_tnh_b2 = 0.0, lt_tnh_t10al = 0.0;
        double lt_tnh_t10c = 0.0, lt_tnh_t10cw = 0.0, lt_tnh_t10ia = 0.0, lt_tnh_t10q = 0.0, lt_tnh_t10sw = 0.0, lt_tnh_t10w = 0.0;
        double lt_tnh_z1 = 0.0, lt_tnh_z2 = 0.0, lt_tnl_a1 = 0.0, lt_tnl_a2 = 0.0, lt_tnl_b0 = 0.0, lt_tnl_b1 = 0.0;
        double lt_tnl_b2 = 0.0, lt_tnl_t11al = 0.0, lt_tnl_t11c = 0.0, lt_tnl_t11cw = 0.0, lt_tnl_t11ia = 0.0, lt_tnl_t11q = 0.0;
        double lt_tnl_t11sw = 0.0, lt_tnl_t11w = 0.0, lt_tnl_z1 = 0.0, lt_tnl_z2 = 0.0, lt_tnp = 0.0, lt_trlast = 0.0;
        double lt_upper = 0.0, lt_vel = 0.0, lt_vg = 0.0, oh_ac = 0.0, oh_active = 0.0, oh_ae = 0.0;
        double oh_bac = 0.0, oh_bae = 0.0, oh_bd = 0.0, oh_bg = 0.0, oh_chk = 0.0, oh_chkg = 0.0;
        double oh_ckc = 0.0, oh_cke = 0.0, oh_dcx = 0.0, oh_dcy = 0.0, oh_dec = 0.0, oh_env = 0.0;
        double oh_f = 0.0, oh_fdc = 0.0, oh_fe = 0.0, oh_fi = 0.0, oh_fire = 0.0, oh_fr = 0.0;
        double oh_gff = 0.0, oh_hp_a1 = 0.0, oh_hp_a2 = 0.0, oh_hp_b0 = 0.0, oh_hp_b1 = 0.0, oh_hp_b2 = 0.0;
        double oh_hp_t24al = 0.0, oh_hp_t24c = 0.0, oh_hp_t24cw = 0.0, oh_hp_t24ia = 0.0, oh_hp_t24q = 0.0, oh_hp_t24sw = 0.0;
        double oh_hp_t24w = 0.0, oh_hp_z1 = 0.0, oh_hp_z2 = 0.0, oh_ic = 0.0, oh_igf = 0.0, oh_left = 0.0;
        double oh_lg = 0.0, oh_lp_a1 = 0.0, oh_lp_a2 = 0.0, oh_lp_b0 = 0.0, oh_lp_b1 = 0.0, oh_lp_b2 = 0.0;
        double oh_lp_t25al = 0.0, oh_lp_t25c = 0.0, oh_lp_t25cw = 0.0, oh_lp_t25ia = 0.0, oh_lp_t25q = 0.0, oh_lp_t25sw = 0.0;
        double oh_lp_t25w = 0.0, oh_lp_z1 = 0.0, oh_lp_z2 = 0.0, oh_lv = 0.0, oh_mg = 0.0, oh_mt = 0.0;
        double oh_n1 = 0.0, oh_nd = 0.0, oh_nf = 0.0, oh_nin = 0.0, oh_nout = 0.0, oh_nx = 0.0;
        double oh_nz = 0.0, oh_o = 0.0, oh_out = 0.0, oh_pv = 0.0, oh_rr = 0.0, oh_sdc = 0.0;
        double oh_se = 0.0, oh_sq = 0.0, oh_src = 0.0, oh_ton = 0.0, oh_tp = 0.0, oh_trlast = 0.0;
        double oh_vel = 0.0, oh_vg = 0.0, oh_wa = 0.0, oh_wd = 0.0, oh_wn = 0.0, oh_wv = 0.0;
        double oh_y = 0.0, sd_active = 0.0, sd_ap = 0.0, sd_att = 0.0, sd_b1_a1 = 0.0, sd_b1_a2 = 0.0;
        double sd_b1_b0 = 0.0, sd_b1_b1 = 0.0, sd_b1_b2 = 0.0, sd_b1_t2al = 0.0, sd_b1_t2c = 0.0, sd_b1_t2cw = 0.0;
        double sd_b1_t2ia = 0.0, sd_b1_t2q = 0.0, sd_b1_t2sw = 0.0, sd_b1_t2w = 0.0, sd_b1_z1 = 0.0, sd_b1_z2 = 0.0;
        double sd_b2_a1 = 0.0, sd_b2_a2 = 0.0, sd_b2_b0 = 0.0, sd_b2_b1 = 0.0, sd_b2_b2 = 0.0, sd_b2_t3al = 0.0;
        double sd_b2_t3c = 0.0, sd_b2_t3cw = 0.0, sd_b2_t3ia = 0.0, sd_b2_t3q = 0.0, sd_b2_t3sw = 0.0, sd_b2_t3w = 0.0;
        double sd_b2_z1 = 0.0, sd_b2_z2 = 0.0, sd_body = 0.0, sd_bph = 0.0, sd_col = 0.0, sd_dec = 0.0;
        double sd_env = 0.0, sd_fi = 0.0, sd_fire = 0.0, sd_fr = 0.0, sd_gate = 0.0, sd_gfade = 0.0;
        double sd_ghold = 0.0, sd_imp = 0.0, sd_lpa = 0.0, sd_lpz = 0.0, sd_mg = 0.0, sd_mt = 0.0;
        double sd_nf = 0.0, sd_ngain = 0.0, sd_no = 0.0, sd_nz = 0.0, sd_o1 = 0.0, sd_o2 = 0.0;
        double sd_out = 0.0, sd_pr = 0.0, sd_prog = 0.0, sd_rem = 0.0, sd_renv = 0.0, sd_rf = 0.0;
        double sd_rph = 0.0, sd_sel = 0.0, sd_snap = 0.0, sd_sq = 0.0, sd_tenv = 0.0, sd_tm = 0.0;
        double sd_trlast = 0.0, sd_tt = 0.0, sd_vel = 0.0, sd_vg = 0.0, sd_w = 0.0, sd_wat = 0.0;
        double sd_wenv = 0.0, seq_acc = 0.0, seq_b = 0.0, seq_hit = 0.0, seq_s = 0.0, seq_vel = 0.0;
        double ticklast = 0.0, total = 0.0;
        fs = fs_;
        i = 0.0;
        j = 0.0;
        k = 0.0;
        ticklast = h_ticklast;
        bd_mg = h_bd_mg;
        sd_mg = h_sd_mg;
        lt_mg = h_lt_mg;
        ht_mg = h_ht_mg;
        cp_mg = h_cp_mg;
        ch_mg = h_ch_mg;
        oh_mg = h_oh_mg;
        cy_mg = h_cy_mg;
        bd_active = h_bd_active;
        bd_trlast = h_bd_trlast;
        bd_vg = h_bd_vg;
        bd_sil = h_bd_sil;
        bd_ph = h_bd_ph;
        bd_hz = h_bd_hz;
        bd_tg = h_bd_tg;
        bd_amp = h_bd_amp;
        bd_ampc = h_bd_ampc;
        bd_pc = h_bd_pc;
        bd_ce = h_bd_ce;
        bd_cc = h_bd_cc;
        bd_ie = h_bd_ie;
        bd_ic = h_bd_ic;
        bd_clz = h_bd_clz;
        bd_cla = h_bd_cla;
        bd_blz = h_bd_blz;
        bd_bla = h_bd_bla;
        bd_ilz = h_bd_ilz;
        bd_ila = h_bd_ila;
        bd_dcx = h_bd_dcx;
        bd_dcy = h_bd_dcy;
        bd_last = h_bd_last;
        bd_nin = h_bd_nin;
        bd_nout = h_bd_nout;
        bd_amt = h_bd_amt;
        bd_hp_b0 = h_bd_hp_b0;
        bd_hp_b1 = h_bd_hp_b1;
        bd_hp_b2 = h_bd_hp_b2;
        bd_hp_a1 = h_bd_hp_a1;
        bd_hp_a2 = h_bd_hp_a2;
        bd_hp_z1 = h_bd_hp_z1;
        bd_hp_z2 = h_bd_hp_z2;
        bd_out = 0.0;
        sd_active = h_sd_active;
        sd_trlast = h_sd_trlast;
        sd_vg = h_sd_vg;
        sd_fi = h_sd_fi;
        sd_nf = h_sd_nf;
        sd_bph = h_sd_bph;
        sd_rph = h_sd_rph;
        sd_dec = h_sd_dec;
        sd_pr = h_sd_pr;
        sd_snap = h_sd_snap;
        sd_col = h_sd_col;
        sd_sel = h_sd_sel;
        sd_ghold = h_sd_ghold;
        sd_gfade = h_sd_gfade;
        sd_ngain = h_sd_ngain;
        sd_lpz = h_sd_lpz;
        sd_lpa = h_sd_lpa;
        sd_b1_b0 = h_sd_b1_b0;
        sd_b1_b1 = h_sd_b1_b1;
        sd_b1_b2 = h_sd_b1_b2;
        sd_b1_a1 = h_sd_b1_a1;
        sd_b1_a2 = h_sd_b1_a2;
        sd_b1_z1 = h_sd_b1_z1;
        sd_b1_z2 = h_sd_b1_z2;
        sd_b2_b0 = h_sd_b2_b0;
        sd_b2_b1 = h_sd_b2_b1;
        sd_b2_b2 = h_sd_b2_b2;
        sd_b2_a1 = h_sd_b2_a1;
        sd_b2_a2 = h_sd_b2_a2;
        sd_b2_z1 = h_sd_b2_z1;
        sd_b2_z2 = h_sd_b2_z2;
        sd_out = 0.0;
        lt_active = h_lt_active;
        lt_trlast = h_lt_trlast;
        lt_vg = h_lt_vg;
        lt_mhz = h_lt_mhz;
        lt_glide = h_lt_glide;
        lt_gpole = h_lt_gpole;
        lt_lhz = h_lt_lhz;
        lt_shz = h_lt_shz;
        lt_mph = h_lt_mph;
        lt_lph = h_lt_lph;
        lt_sph = h_lt_sph;
        lt_me = h_lt_me;
        lt_le = h_lt_le;
        lt_se = h_lt_se;
        lt_sne = h_lt_sne;
        lt_bue = h_lt_bue;
        lt_tne = h_lt_tne;
        lt_mpole = h_lt_mpole;
        lt_lpole = h_lt_lpole;
        lt_spole = h_lt_spole;
        lt_snp = h_lt_snp;
        lt_bup = h_lt_bup;
        lt_tnp = h_lt_tnp;
        lt_br = h_lt_br;
        lt_brc = h_lt_brc;
        lt_sr = h_lt_sr;
        lt_src = h_lt_src;
        lt_snr = h_lt_snr;
        lt_snrc = h_lt_snrc;
        lt_nr = h_lt_nr;
        lt_nrc = h_lt_nrc;
        lt_lnh_b0 = h_lt_lnh_b0;
        lt_lnh_b1 = h_lt_lnh_b1;
        lt_lnh_b2 = h_lt_lnh_b2;
        lt_lnh_a1 = h_lt_lnh_a1;
        lt_lnh_a2 = h_lt_lnh_a2;
        lt_lnh_z1 = h_lt_lnh_z1;
        lt_lnh_z2 = h_lt_lnh_z2;
        lt_lnl_b0 = h_lt_lnl_b0;
        lt_lnl_b1 = h_lt_lnl_b1;
        lt_lnl_b2 = h_lt_lnl_b2;
        lt_lnl_a1 = h_lt_lnl_a1;
        lt_lnl_a2 = h_lt_lnl_a2;
        lt_lnl_z1 = h_lt_lnl_z1;
        lt_lnl_z2 = h_lt_lnl_z2;
        lt_hnh_b0 = h_lt_hnh_b0;
        lt_hnh_b1 = h_lt_hnh_b1;
        lt_hnh_b2 = h_lt_hnh_b2;
        lt_hnh_a1 = h_lt_hnh_a1;
        lt_hnh_a2 = h_lt_hnh_a2;
        lt_hnh_z1 = h_lt_hnh_z1;
        lt_hnh_z2 = h_lt_hnh_z2;
        lt_hnl_b0 = h_lt_hnl_b0;
        lt_hnl_b1 = h_lt_hnl_b1;
        lt_hnl_b2 = h_lt_hnl_b2;
        lt_hnl_a1 = h_lt_hnl_a1;
        lt_hnl_a2 = h_lt_hnl_a2;
        lt_hnl_z1 = h_lt_hnl_z1;
        lt_hnl_z2 = h_lt_hnl_z2;
        lt_tnh_b0 = h_lt_tnh_b0;
        lt_tnh_b1 = h_lt_tnh_b1;
        lt_tnh_b2 = h_lt_tnh_b2;
        lt_tnh_a1 = h_lt_tnh_a1;
        lt_tnh_a2 = h_lt_tnh_a2;
        lt_tnh_z1 = h_lt_tnh_z1;
        lt_tnh_z2 = h_lt_tnh_z2;
        lt_tnl_b0 = h_lt_tnl_b0;
        lt_tnl_b1 = h_lt_tnl_b1;
        lt_tnl_b2 = h_lt_tnl_b2;
        lt_tnl_a1 = h_lt_tnl_a1;
        lt_tnl_a2 = h_lt_tnl_a2;
        lt_tnl_z1 = h_lt_tnl_z1;
        lt_tnl_z2 = h_lt_tnl_z2;
        lt_fsh_b0 = h_lt_fsh_b0;
        lt_fsh_b1 = h_lt_fsh_b1;
        lt_fsh_b2 = h_lt_fsh_b2;
        lt_fsh_a1 = h_lt_fsh_a1;
        lt_fsh_a2 = h_lt_fsh_a2;
        lt_fsh_z1 = h_lt_fsh_z1;
        lt_fsh_z2 = h_lt_fsh_z2;
        lt_fsl_b0 = h_lt_fsl_b0;
        lt_fsl_b1 = h_lt_fsl_b1;
        lt_fsl_b2 = h_lt_fsl_b2;
        lt_fsl_a1 = h_lt_fsl_a1;
        lt_fsl_a2 = h_lt_fsl_a2;
        lt_fsl_z1 = h_lt_fsl_z1;
        lt_fsl_z2 = h_lt_fsl_z2;
        lt_out = 0.0;
        ht_active = h_ht_active;
        ht_trlast = h_ht_trlast;
        ht_vg = h_ht_vg;
        ht_mhz = h_ht_mhz;
        ht_glide = h_ht_glide;
        ht_gpole = h_ht_gpole;
        ht_lhz = h_ht_lhz;
        ht_shz = h_ht_shz;
        ht_mph = h_ht_mph;
        ht_lph = h_ht_lph;
        ht_sph = h_ht_sph;
        ht_me = h_ht_me;
        ht_le = h_ht_le;
        ht_se = h_ht_se;
        ht_sne = h_ht_sne;
        ht_bue = h_ht_bue;
        ht_tne = h_ht_tne;
        ht_mpole = h_ht_mpole;
        ht_lpole = h_ht_lpole;
        ht_spole = h_ht_spole;
        ht_snp = h_ht_snp;
        ht_bup = h_ht_bup;
        ht_tnp = h_ht_tnp;
        ht_br = h_ht_br;
        ht_brc = h_ht_brc;
        ht_sr = h_ht_sr;
        ht_src = h_ht_src;
        ht_snr = h_ht_snr;
        ht_snrc = h_ht_snrc;
        ht_nr = h_ht_nr;
        ht_nrc = h_ht_nrc;
        ht_uph0 = h_ht_uph0;
        ht_uhz0 = h_ht_uhz0;
        ht_ue0 = h_ht_ue0;
        ht_up0 = h_ht_up0;
        ht_uph1 = h_ht_uph1;
        ht_uhz1 = h_ht_uhz1;
        ht_ue1 = h_ht_ue1;
        ht_up1 = h_ht_up1;
        ht_uph2 = h_ht_uph2;
        ht_uhz2 = h_ht_uhz2;
        ht_ue2 = h_ht_ue2;
        ht_up2 = h_ht_up2;
        ht_uph3 = h_ht_uph3;
        ht_uhz3 = h_ht_uhz3;
        ht_ue3 = h_ht_ue3;
        ht_up3 = h_ht_up3;
        ht_lnh_b0 = h_ht_lnh_b0;
        ht_lnh_b1 = h_ht_lnh_b1;
        ht_lnh_b2 = h_ht_lnh_b2;
        ht_lnh_a1 = h_ht_lnh_a1;
        ht_lnh_a2 = h_ht_lnh_a2;
        ht_lnh_z1 = h_ht_lnh_z1;
        ht_lnh_z2 = h_ht_lnh_z2;
        ht_lnl_b0 = h_ht_lnl_b0;
        ht_lnl_b1 = h_ht_lnl_b1;
        ht_lnl_b2 = h_ht_lnl_b2;
        ht_lnl_a1 = h_ht_lnl_a1;
        ht_lnl_a2 = h_ht_lnl_a2;
        ht_lnl_z1 = h_ht_lnl_z1;
        ht_lnl_z2 = h_ht_lnl_z2;
        ht_hnh_b0 = h_ht_hnh_b0;
        ht_hnh_b1 = h_ht_hnh_b1;
        ht_hnh_b2 = h_ht_hnh_b2;
        ht_hnh_a1 = h_ht_hnh_a1;
        ht_hnh_a2 = h_ht_hnh_a2;
        ht_hnh_z1 = h_ht_hnh_z1;
        ht_hnh_z2 = h_ht_hnh_z2;
        ht_hnl_b0 = h_ht_hnl_b0;
        ht_hnl_b1 = h_ht_hnl_b1;
        ht_hnl_b2 = h_ht_hnl_b2;
        ht_hnl_a1 = h_ht_hnl_a1;
        ht_hnl_a2 = h_ht_hnl_a2;
        ht_hnl_z1 = h_ht_hnl_z1;
        ht_hnl_z2 = h_ht_hnl_z2;
        ht_tnh_b0 = h_ht_tnh_b0;
        ht_tnh_b1 = h_ht_tnh_b1;
        ht_tnh_b2 = h_ht_tnh_b2;
        ht_tnh_a1 = h_ht_tnh_a1;
        ht_tnh_a2 = h_ht_tnh_a2;
        ht_tnh_z1 = h_ht_tnh_z1;
        ht_tnh_z2 = h_ht_tnh_z2;
        ht_tnl_b0 = h_ht_tnl_b0;
        ht_tnl_b1 = h_ht_tnl_b1;
        ht_tnl_b2 = h_ht_tnl_b2;
        ht_tnl_a1 = h_ht_tnl_a1;
        ht_tnl_a2 = h_ht_tnl_a2;
        ht_tnl_z1 = h_ht_tnl_z1;
        ht_tnl_z2 = h_ht_tnl_z2;
        ht_out = 0.0;
        cp_active = h_cp_active;
        cp_trlast = h_cp_trlast;
        cp_vg = h_cp_vg;
        cp_si = h_cp_si;
        cp_maxs = h_cp_maxs;
        cp_dec = h_cp_dec;
        cp_air = h_cp_air;
        cp_nlg = h_cp_nlg;
        cp_ahc = h_cp_ahc;
        cp_ahi = h_cp_ahi;
        cp_aho = h_cp_aho;
        cp_tapn = h_cp_tapn;
        cp_pos = h_cp_pos;
        cp_bl = h_cp_bl;
        cp_la1_b0 = h_cp_la1_b0;
        cp_la1_b1 = h_cp_la1_b1;
        cp_la1_b2 = h_cp_la1_b2;
        cp_la1_a1 = h_cp_la1_a1;
        cp_la1_a2 = h_cp_la1_a2;
        cp_la1_z1 = h_cp_la1_z1;
        cp_la1_z2 = h_cp_la1_z2;
        cp_la2_b0 = h_cp_la2_b0;
        cp_la2_b1 = h_cp_la2_b1;
        cp_la2_b2 = h_cp_la2_b2;
        cp_la2_a1 = h_cp_la2_a1;
        cp_la2_a2 = h_cp_la2_a2;
        cp_la2_z1 = h_cp_la2_z1;
        cp_la2_z2 = h_cp_la2_z2;
        cp_lb1_b0 = h_cp_lb1_b0;
        cp_lb1_b1 = h_cp_lb1_b1;
        cp_lb1_b2 = h_cp_lb1_b2;
        cp_lb1_a1 = h_cp_lb1_a1;
        cp_lb1_a2 = h_cp_lb1_a2;
        cp_lb1_z1 = h_cp_lb1_z1;
        cp_lb1_z2 = h_cp_lb1_z2;
        cp_lb2_b0 = h_cp_lb2_b0;
        cp_lb2_b1 = h_cp_lb2_b1;
        cp_lb2_b2 = h_cp_lb2_b2;
        cp_lb2_a1 = h_cp_lb2_a1;
        cp_lb2_a2 = h_cp_lb2_a2;
        cp_lb2_z1 = h_cp_lb2_z1;
        cp_lb2_z2 = h_cp_lb2_z2;
        cp_out = 0.0;
        ch_active = h_ch_active;
        ch_trlast = h_ch_trlast;
        ch_vg = h_ch_vg;
        ch_fi = h_ch_fi;
        ch_nf = h_ch_nf;
        ch_gff = h_ch_gff;
        ch_igf = h_ch_igf;
        ch_bae = h_ch_bae;
        ch_bac = h_ch_bac;
        ch_cke = h_ch_cke;
        ch_ckc = h_ch_ckc;
        ch_dcx = h_ch_dcx;
        ch_dcy = h_ch_dcy;
        ch_wa = h_ch_wa;
        ch_wd = h_ch_wd;
        ch_fe = h_ch_fe;
        ch_se = h_ch_se;
        ch_fdc = h_ch_fdc;
        ch_sdc = h_ch_sdc;
        ch_ae = h_ch_ae;
        ch_ac = h_ch_ac;
        ch_nin = h_ch_nin;
        ch_nout = h_ch_nout;
        ch_hp_b0 = h_ch_hp_b0;
        ch_hp_b1 = h_ch_hp_b1;
        ch_hp_b2 = h_ch_hp_b2;
        ch_hp_a1 = h_ch_hp_a1;
        ch_hp_a2 = h_ch_hp_a2;
        ch_hp_z1 = h_ch_hp_z1;
        ch_hp_z2 = h_ch_hp_z2;
        ch_lp_b0 = h_ch_lp_b0;
        ch_lp_b1 = h_ch_lp_b1;
        ch_lp_b2 = h_ch_lp_b2;
        ch_lp_a1 = h_ch_lp_a1;
        ch_lp_a2 = h_ch_lp_a2;
        ch_lp_z1 = h_ch_lp_z1;
        ch_lp_z2 = h_ch_lp_z2;
        ch_out = 0.0;
        oh_active = h_oh_active;
        oh_trlast = h_oh_trlast;
        oh_vg = h_oh_vg;
        oh_fi = h_oh_fi;
        oh_nf = h_oh_nf;
        oh_gff = h_oh_gff;
        oh_igf = h_oh_igf;
        oh_bae = h_oh_bae;
        oh_bac = h_oh_bac;
        oh_cke = h_oh_cke;
        oh_ckc = h_oh_ckc;
        oh_dcx = h_oh_dcx;
        oh_dcy = h_oh_dcy;
        oh_wa = h_oh_wa;
        oh_wd = h_oh_wd;
        oh_fe = h_oh_fe;
        oh_se = h_oh_se;
        oh_fdc = h_oh_fdc;
        oh_sdc = h_oh_sdc;
        oh_ae = h_oh_ae;
        oh_ac = h_oh_ac;
        oh_nin = h_oh_nin;
        oh_nout = h_oh_nout;
        oh_hp_b0 = h_oh_hp_b0;
        oh_hp_b1 = h_oh_hp_b1;
        oh_hp_b2 = h_oh_hp_b2;
        oh_hp_a1 = h_oh_hp_a1;
        oh_hp_a2 = h_oh_hp_a2;
        oh_hp_z1 = h_oh_hp_z1;
        oh_hp_z2 = h_oh_hp_z2;
        oh_lp_b0 = h_oh_lp_b0;
        oh_lp_b1 = h_oh_lp_b1;
        oh_lp_b2 = h_oh_lp_b2;
        oh_lp_a1 = h_oh_lp_a1;
        oh_lp_a2 = h_oh_lp_a2;
        oh_lp_z1 = h_oh_lp_z1;
        oh_lp_z2 = h_oh_lp_z2;
        oh_chk = h_oh_chk;
        oh_chkg = h_oh_chkg;
        oh_out = 0.0;
        cy_active = h_cy_active;
        cy_trlast = h_cy_trlast;
        cy_vg = h_cy_vg;
        cy_p0 = h_cy_p0;
        cy_p1 = h_cy_p1;
        cy_p2 = h_cy_p2;
        cy_p3 = h_cy_p3;
        cy_p4 = h_cy_p4;
        cy_p5 = h_cy_p5;
        cy_rt = h_cy_rt;
        cy_eh = h_cy_eh;
        cy_em = h_cy_em;
        cy_ehc = h_cy_ehc;
        cy_emc = h_cy_emc;
        cy_rise = h_cy_rise;
        cy_risec = h_cy_risec;
        cy_n = h_cy_n;
        cy_gh = h_cy_gh;
        cy_gm = h_cy_gm;
        cy_dec = h_cy_dec;
        cy_h1_b0 = h_cy_h1_b0;
        cy_h1_b1 = h_cy_h1_b1;
        cy_h1_b2 = h_cy_h1_b2;
        cy_h1_a1 = h_cy_h1_a1;
        cy_h1_a2 = h_cy_h1_a2;
        cy_h1_z1 = h_cy_h1_z1;
        cy_h1_z2 = h_cy_h1_z2;
        cy_h2_b0 = h_cy_h2_b0;
        cy_h2_b1 = h_cy_h2_b1;
        cy_h2_b2 = h_cy_h2_b2;
        cy_h2_a1 = h_cy_h2_a1;
        cy_h2_a2 = h_cy_h2_a2;
        cy_h2_z1 = h_cy_h2_z1;
        cy_h2_z2 = h_cy_h2_z2;
        cy_m1_b0 = h_cy_m1_b0;
        cy_m1_b1 = h_cy_m1_b1;
        cy_m1_b2 = h_cy_m1_b2;
        cy_m1_a1 = h_cy_m1_a1;
        cy_m1_a2 = h_cy_m1_a2;
        cy_m1_z1 = h_cy_m1_z1;
        cy_m1_z2 = h_cy_m1_z2;
        cy_m2_b0 = h_cy_m2_b0;
        cy_m2_b1 = h_cy_m2_b1;
        cy_m2_b2 = h_cy_m2_b2;
        cy_m2_a1 = h_cy_m2_a1;
        cy_m2_a2 = h_cy_m2_a2;
        cy_m2_z1 = h_cy_m2_z1;
        cy_m2_z2 = h_cy_m2_z2;
        cy_hp_b0 = h_cy_hp_b0;
        cy_hp_b1 = h_cy_hp_b1;
        cy_hp_b2 = h_cy_hp_b2;
        cy_hp_a1 = h_cy_hp_a1;
        cy_hp_a2 = h_cy_hp_a2;
        cy_hp_z1 = h_cy_hp_z1;
        cy_hp_z2 = h_cy_hp_z2;
        cy_out = 0.0;
        acc = accent * 0.01;
        seq_hit = 0.0;
        seq_s = 0.0;
        if (seqtick != ticklast) {
          ticklast = seqtick;
          if (seqon > 0.0) { seq_hit = 1.0; seq_s = stp; }
        }
        seq_b = std::pow(2.0, seq_s);
        seq_acc = seq_hit * (std::floor(m_ac / seq_b) - 2.0 * std::floor(m_ac / (2.0 * seq_b)));
        seq_vel = 100.0;
        if (seq_acc > 0.0) { seq_vel = 127.0; }
        bd_sq = seq_hit * (std::floor(m_bd / seq_b) - 2.0 * std::floor(m_bd / (2.0 * seq_b)));
        sd_sq = seq_hit * (std::floor(m_sd / seq_b) - 2.0 * std::floor(m_sd / (2.0 * seq_b)));
        lt_sq = seq_hit * (std::floor(m_lt / seq_b) - 2.0 * std::floor(m_lt / (2.0 * seq_b)));
        ht_sq = seq_hit * (std::floor(m_ht / seq_b) - 2.0 * std::floor(m_ht / (2.0 * seq_b)));
        cp_sq = seq_hit * (std::floor(m_cp / seq_b) - 2.0 * std::floor(m_cp / (2.0 * seq_b)));
        ch_sq = seq_hit * (std::floor(m_ch / seq_b) - 2.0 * std::floor(m_ch / (2.0 * seq_b)));
        oh_sq = seq_hit * (std::floor(m_oh / seq_b) - 2.0 * std::floor(m_oh / (2.0 * seq_b)));
        cy_sq = seq_hit * (std::floor(m_cy / seq_b) - 2.0 * std::floor(m_cy / (2.0 * seq_b)));
        if ((tr_ch != ch_trlast || ch_sq > 0.0) && oh_active > 0.0) { oh_chk = dcoef(fs, 0.004); }
        // ---- bd ----
        bd_fire = 0.0;
        bd_vel = 0.0;
        if (tr_bd != bd_trlast) {
          bd_trlast = tr_bd;
          bd_vel = tr_bd - 128.0 * std::floor(tr_bd / 128.0);
          bd_fire = 1.0;
        }
        if (bd_sq > 0.0) { bd_vel = gmax(bd_vel, seq_vel); bd_fire = 1.0; }
        if (bd_fire > 0.0) {
          bd_vg = velgain(bd_vel, acc);
          bd_T = gclamp(bd_attack * 0.01, 0.0, 1.0);
          bd_D = gclamp(bd_decay * 0.01, 0.0, 1.0);
          bd_dec = -1.75 + 4.75 * bd_D * bd_D;
          bd_amt = 0.1 + bd_T * 0.5;
          bd_ratio = std::pow(2.0, bd_tune / 12.0);
          bd_start = (120.0 + (0.28 - 0.5) * 16.0) * bd_ratio;
          bd_end = (53.0 + (0.28 - 0.5) * 8.0) * bd_ratio;
          bd_ts = gclamp(bd_amt, 0.0, 1.5);
          bd_thud = gclamp(bd_ts * bd_ts, 0.0, 1.5);
          bd_pd = 0.022 + (0.008 - 0.022) * gclamp(bd_thud, 0.0, 1.0);
          bd_amp = 0.92 + bd_thud * 0.22 + 0.08;
          bd_ph = -0.33 * 3.141592653589793;
          bd_hz = bd_start * (1.0 + bd_thud * 0.28);
          bd_tg = bd_end;
          bd_ampc = dcoef(fs, 0.22 + bd_dec * 0.12);
          bd_pc = 1.0 - std::exp(-1.0 / (fs * gmax(0.0005, bd_pd)));
          bd_cc = dcoef(fs, 0.002 + bd_amt * 0.0045 + bd_amt * 0.004);
          bd_ce = gmax(bd_ce, 0.02 + bd_amt * 0.06 + bd_amt * 0.28 + 0.02);
          bd_ic = dcoef(fs, 0.0008 + bd_amt * 0.008);
          bd_ie = gmax(bd_ie, 0.02 + bd_amt * 0.55 + bd_thud * 0.08 + 0.04);
          bd_bla = lpcoef(fs, 620.0 + (1200.0 - 620.0) * 0.34);
          bd_cla = lpcoef(fs, 1400.0);
          bd_ila = lpcoef(fs, 2600.0);
          bd_hp_t1c = gclamp(1350.0, 10.0, fs * 0.45);
          bd_hp_t1q = gmax(0.05, 0.707);
          bd_hp_t1w = 6.283185307179586 * bd_hp_t1c / fs;
          bd_hp_t1cw = std::cos(bd_hp_t1w);
          bd_hp_t1sw = std::sin(bd_hp_t1w);
          bd_hp_t1al = bd_hp_t1sw / (2.0 * bd_hp_t1q);
          bd_hp_t1ia = 1.0 / (1.0 + bd_hp_t1al);
          bd_hp_b0 = (1.0 + bd_hp_t1cw) * 0.5 * bd_hp_t1ia;
          bd_hp_b1 = -(1.0 + bd_hp_t1cw) * bd_hp_t1ia;
          bd_hp_b2 = bd_hp_b0;
          bd_hp_a1 = -2.0 * bd_hp_t1cw * bd_hp_t1ia;
          bd_hp_a2 = (1.0 - bd_hp_t1al) * bd_hp_t1ia;
          bd_last = 0.0;
          bd_active = 1.0;
          bd_sil = 0.0;
        }
        if (bd_active > 0.0) {
          bd_hz = bd_hz + (bd_tg - bd_hz) * bd_pc;
          bd_ph = bd_ph + 6.283185307179586 * bd_hz / fs;
          if (bd_ph >= 6.283185307179586) { bd_ph = bd_ph - 6.283185307179586; }
          bd_raw = std::sin(bd_ph) * bd_amp;
          bd_amp = bd_amp * bd_ampc;
          if (bd_amp < 0.000001) { bd_amp = 0.0; }
          bd_blz = bd_blz + bd_bla * (bd_raw - bd_blz);
          bd_nx = noise_();
          bd_n = bd_nx - bd_nin + 0.995 * bd_nout;
          bd_nin = bd_nx;
          bd_nout = bd_n;
          bd_clz = bd_clz + bd_cla * (bd_n - bd_clz);
          bd_ce = bd_ce * bd_cc;
          if (bd_ce < 0.000001) { bd_ce = 0.0; }
          bd_dl = bd_raw - bd_last;
          bd_last = bd_raw;
          bd_hpo = bd_hp_b0 * bd_dl + bd_hp_z1;
          bd_hp_z1 = bd_hp_b1 * bd_dl - bd_hp_a1 * bd_hpo + bd_hp_z2;
          bd_hp_z2 = bd_hp_b2 * bd_dl - bd_hp_a2 * bd_hpo;
          bd_ilz = bd_ilz + bd_ila * (bd_hpo - bd_ilz);
          bd_ie = bd_ie * bd_ic;
          if (bd_ie < 0.000001) { bd_ie = 0.0; }
          bd_sum = bd_blz * 0.938 + bd_clz * bd_ce * (0.06 + bd_amt * 0.9) + bd_ilz * bd_ie * (0.08 + bd_amt * 2.2);
          bd_sh = std::tanh(bd_sum * 1.1416);
          bd_y = bd_sh - bd_dcx + 0.995 * bd_dcy;
          bd_dcx = bd_sh;
          bd_dcy = bd_y;
          bd_out = bd_y * 0.7;
          if (bd_amp > 0.000001 || bd_ce > 0.000001 || bd_ie > 0.000001) { bd_sil = 0.0; }
          else if (std::fabs(bd_out) < 0.00001) { bd_sil = bd_sil + 1.0; if (bd_sil >= 32.0) { bd_active = 0.0; } }
          else { bd_sil = 0.0; }
        }
        // ---- sd ----
        sd_fire = 0.0;
        sd_vel = 0.0;
        if (tr_sd != sd_trlast) {
          sd_trlast = tr_sd;
          sd_vel = tr_sd - 128.0 * std::floor(tr_sd / 128.0);
          sd_fire = 1.0;
        }
        if (sd_sq > 0.0) { sd_vel = gmax(sd_vel, seq_vel); sd_fire = 1.0; }
        if (sd_fire > 0.0) {
          sd_vg = velgain(sd_vel, acc);
          sd_dec = gclamp(sd_decay * 0.01, 0.01, 1.0);
          sd_pr = gclamp(std::pow(2.0, sd_tune / 12.0), 0.25, 4.0);
          sd_snap = gclamp(sd_snappy * 0.01, 0.0, 1.0);
          sd_col = 1.0;
          sd_sel = 0.370136054422 * sd_dec;
          if (sd_dec >= 0.999) { sd_sel = 0.370136054422; }
          sd_rem = gmax(0.0, 0.370136054422 - sd_sel);
          sd_gfade = gmin(sd_sel, sd_rem);
          sd_ghold = gmax(0.0, sd_sel - sd_gfade);
          sd_nf = gmax(1.0, std::ceil(sd_sel * fs));
          sd_fi = 0.0;
          sd_bph = 1.1711004;
          sd_rph = -1.1;
          sd_b1_t2c = gclamp(2998.628 * sd_col, 10.0, fs * 0.45);
          sd_b1_t2q = gmax(0.05, 1.2771896);
          sd_b1_t2w = 6.283185307179586 * sd_b1_t2c / fs;
          sd_b1_t2cw = std::cos(sd_b1_t2w);
          sd_b1_t2sw = std::sin(sd_b1_t2w);
          sd_b1_t2al = sd_b1_t2sw / (2.0 * sd_b1_t2q);
          sd_b1_t2ia = 1.0 / (1.0 + sd_b1_t2al);
          sd_b1_b0 = 0.5 * sd_b1_t2sw * sd_b1_t2ia;
          sd_b1_b1 = 0.0;
          sd_b1_b2 = -0.5 * sd_b1_t2sw * sd_b1_t2ia;
          sd_b1_a1 = -2.0 * sd_b1_t2cw * sd_b1_t2ia;
          sd_b1_a2 = (1.0 - sd_b1_t2al) * sd_b1_t2ia;
          sd_b1_z1 = 0.0;
          sd_b1_z2 = 0.0;
          sd_b2_t3c = gclamp(4596.2905 * sd_col, 10.0, fs * 0.45);
          sd_b2_t3q = gmax(0.05, 0.30369505);
          sd_b2_t3w = 6.283185307179586 * sd_b2_t3c / fs;
          sd_b2_t3cw = std::cos(sd_b2_t3w);
          sd_b2_t3sw = std::sin(sd_b2_t3w);
          sd_b2_t3al = sd_b2_t3sw / (2.0 * sd_b2_t3q);
          sd_b2_t3ia = 1.0 / (1.0 + sd_b2_t3al);
          sd_b2_b0 = 0.5 * sd_b2_t3sw * sd_b2_t3ia;
          sd_b2_b1 = 0.0;
          sd_b2_b2 = -0.5 * sd_b2_t3sw * sd_b2_t3ia;
          sd_b2_a1 = -2.0 * sd_b2_t3cw * sd_b2_t3ia;
          sd_b2_a2 = (1.0 - sd_b2_t3al) * sd_b2_t3ia;
          sd_b2_z1 = 0.0;
          sd_b2_z2 = 0.0;
          sd_lpa = lpcoef(fs, 19677.396 * sd_col);
          sd_lpz = 0.0;
          sd_ngain = std::sqrt(fs / 44100.0);
          sd_active = 1.0;
        }
        if (sd_active > 0.0) {
          if (sd_fi >= sd_nf) { sd_active = 0.0; }
          else {
          sd_tm = sd_fi / fs;
          sd_gate = 1.0;
          if (sd_dec < 0.999 && sd_tm >= sd_ghold) {
            if (sd_gfade <= 0.0) { sd_gate = 0.0; }
            else {
              sd_prog = (sd_tm - sd_ghold) / sd_gfade;
              if (sd_prog >= 1.0) { sd_gate = 0.0; }
              else { sd_gate = gmax(0.0, (1.0 + 0.000015881701) * std::exp(-11.05 * sd_prog) - 0.000015881701); }
            }
          }
          sd_body = 0.0;
          sd_tt = sd_tm * sd_pr;
          if (sd_tt < 0.249795918367 && sd_tm < sd_sel) {
            sd_fr = 201.09442 * sd_pr + 149.23143 * std::exp(-sd_tm / 0.0133309);
            sd_att = std::pow(1.0 - std::exp(-sd_tm / 0.0007633316), 3.2748916);
            sd_env = sd_att * std::exp(-6.9077553 * sd_tt / 0.243);
            sd_tenv = sd_att * std::exp(-6.9077553 * sd_tm / 0.03);
            sd_imp = 0.0;
            if (sd_tm >= 0.0002763113 && sd_tm < 0.0013649489) { sd_imp = -0.3000014 * std::exp(-(sd_tm - 0.0002763113) / 0.0025407782); }
            sd_body = ((0.391 * sd_env + 0.44 * sd_tenv) * std::sin(sd_bph) + sd_imp) * 0.71 * sd_gate;
            sd_bph = wrap2pi(sd_bph + 6.283185307179586 * sd_fr / fs);
          }
          sd_nz = 0.0;
          if (sd_tm < sd_sel) {
            sd_w = noise_();
            sd_o1 = sd_b1_b0 * sd_w + sd_b1_z1;
            sd_b1_z1 = sd_b1_b1 * sd_w - sd_b1_a1 * sd_o1 + sd_b1_z2;
            sd_b1_z2 = sd_b1_b2 * sd_w - sd_b1_a2 * sd_o1;
            sd_o2 = sd_b2_b0 * sd_w + sd_b2_z1;
            sd_b2_z1 = sd_b2_b1 * sd_w - sd_b2_a1 * sd_o2 + sd_b2_z2;
            sd_b2_z2 = sd_b2_b2 * sd_w - sd_b2_a2 * sd_o2;
            sd_lpz = sd_lpz + sd_lpa * ((sd_o1 - 1.597958 * sd_o2) - sd_lpz);
            sd_ap = gmin(sd_tm / 0.0031, 1.0);
            sd_wat = 1.0 - std::pow(1.0 - sd_ap, 1.3);
            sd_wenv = sd_wat * std::exp(-6.9077553 * sd_tm / 0.395);
            sd_no = 1.28 * sd_ngain * sd_wenv * sd_lpz;
            sd_rf = (201.09442 + 149.23143 * std::exp(-sd_tm / 0.0133309)) * sd_col;
            sd_renv = (1.0 - std::exp(-sd_tm / 0.012)) * std::exp(-6.9077553 * sd_tm / 0.395);
            sd_no = sd_no + 0.014 * sd_renv * std::sin(sd_rph);
            sd_rph = wrap2pi(sd_rph + 6.283185307179586 * sd_rf / fs);
            sd_nz = sd_no * sd_gate;
          }
          sd_out = sd_body + sd_nz * sd_snap;
          sd_fi = sd_fi + 1.0;
          if (sd_fi >= sd_nf) { sd_active = 0.0; }
          }
        }
        // ---- lt ----
        lt_fire = 0.0;
        lt_vel = 0.0;
        if (tr_lt != lt_trlast) {
          lt_trlast = tr_lt;
          lt_vel = tr_lt - 128.0 * std::floor(tr_lt / 128.0);
          lt_fire = 1.0;
        }
        if (lt_sq > 0.0) { lt_vel = gmax(lt_vel, seq_vel); lt_fire = 1.0; }
        if (lt_fire > 0.0) {
          lt_vg = velgain(lt_vel, acc);
          lt_dec = gclamp(lt_decay * 0.01, 0.05, 1.0);
          lt_rt = gclamp(std::pow(2.0, lt_tune / 12.0), 0.25, 4.0);
          lt_mhz = 124.435 * lt_rt;
          lt_glide = 39.921 * lt_rt;
          lt_gpole = opcoef(fs, 0.050622);
          lt_lhz = lt_mhz * 1.0;
          lt_shz = 367.0 * lt_rt;
          lt_mph = 2.2826;
          lt_lph = 0.0;
          lt_sph = 0.2;
          lt_me = 1.0;
          lt_le = 1.0;
          lt_se = 1.0;
          lt_sne = 1.0;
          lt_bue = 1.0;
          lt_tne = 1.0;
          lt_mpole = t60coef(fs, 0.3276 * lt_dec);
          lt_lpole = t60coef(fs, 0.1 * lt_dec);
          lt_spole = t60coef(fs, 0.005 * lt_dec);
          lt_snp = t60coef(fs, 0.012 * lt_dec);
          lt_bup = t60coef(fs, 0.08 * lt_dec);
          lt_tnp = t60coef(fs, 0.3 * lt_dec);
          lt_br = 0.0;
          lt_brc = 1.0 - opcoef(fs, 0.00012);
          lt_sr = 0.0;
          lt_src = 1.0 - opcoef(fs, 0.00015);
          lt_snr = 0.0;
          lt_snrc = 1.0 - opcoef(fs, 0.0004);
          lt_nr = 0.0;
          lt_nrc = 1.0 - opcoef(fs, 0.0015);
          lt_lnh_t4c = gclamp(300.0 * lt_rt, 10.0, fs * 0.45);
          lt_lnh_t4q = gmax(0.05, 0.70710678);
          lt_lnh_t4w = 6.283185307179586 * lt_lnh_t4c / fs;
          lt_lnh_t4cw = std::cos(lt_lnh_t4w);
          lt_lnh_t4sw = std::sin(lt_lnh_t4w);
          lt_lnh_t4al = lt_lnh_t4sw / (2.0 * lt_lnh_t4q);
          lt_lnh_t4ia = 1.0 / (1.0 + lt_lnh_t4al);
          lt_lnh_b0 = (1.0 + lt_lnh_t4cw) * 0.5 * lt_lnh_t4ia;
          lt_lnh_b1 = -(1.0 + lt_lnh_t4cw) * lt_lnh_t4ia;
          lt_lnh_b2 = lt_lnh_b0;
          lt_lnh_a1 = -2.0 * lt_lnh_t4cw * lt_lnh_t4ia;
          lt_lnh_a2 = (1.0 - lt_lnh_t4al) * lt_lnh_t4ia;
          lt_lnh_z1 = 0.0;
          lt_lnh_z2 = 0.0;
          lt_lnl_t5c = gclamp(3000.0 * lt_rt, 10.0, fs * 0.45);
          lt_lnl_t5q = gmax(0.05, 0.70710678);
          lt_lnl_t5w = 6.283185307179586 * lt_lnl_t5c / fs;
          lt_lnl_t5cw = std::cos(lt_lnl_t5w);
          lt_lnl_t5sw = std::sin(lt_lnl_t5w);
          lt_lnl_t5al = lt_lnl_t5sw / (2.0 * lt_lnl_t5q);
          lt_lnl_t5ia = 1.0 / (1.0 + lt_lnl_t5al);
          lt_lnl_b0 = (1.0 - lt_lnl_t5cw) * 0.5 * lt_lnl_t5ia;
          lt_lnl_b1 = (1.0 - lt_lnl_t5cw) * lt_lnl_t5ia;
          lt_lnl_b2 = lt_lnl_b0;
          lt_lnl_a1 = -2.0 * lt_lnl_t5cw * lt_lnl_t5ia;
          lt_lnl_a2 = (1.0 - lt_lnl_t5al) * lt_lnl_t5ia;
          lt_lnl_z1 = 0.0;
          lt_lnl_z2 = 0.0;
          lt_hnh_t6c = gclamp(700.0 * lt_rt, 10.0, fs * 0.45);
          lt_hnh_t6q = gmax(0.05, 0.70710678);
          lt_hnh_t6w = 6.283185307179586 * lt_hnh_t6c / fs;
          lt_hnh_t6cw = std::cos(lt_hnh_t6w);
          lt_hnh_t6sw = std::sin(lt_hnh_t6w);
          lt_hnh_t6al = lt_hnh_t6sw / (2.0 * lt_hnh_t6q);
          lt_hnh_t6ia = 1.0 / (1.0 + lt_hnh_t6al);
          lt_hnh_b0 = (1.0 + lt_hnh_t6cw) * 0.5 * lt_hnh_t6ia;
          lt_hnh_b1 = -(1.0 + lt_hnh_t6cw) * lt_hnh_t6ia;
          lt_hnh_b2 = lt_hnh_b0;
          lt_hnh_a1 = -2.0 * lt_hnh_t6cw * lt_hnh_t6ia;
          lt_hnh_a2 = (1.0 - lt_hnh_t6al) * lt_hnh_t6ia;
          lt_hnh_z1 = 0.0;
          lt_hnh_z2 = 0.0;
          lt_hnl_t7c = gclamp(12000.0 * lt_rt, 10.0, fs * 0.45);
          lt_hnl_t7q = gmax(0.05, 0.70710678);
          lt_hnl_t7w = 6.283185307179586 * lt_hnl_t7c / fs;
          lt_hnl_t7cw = std::cos(lt_hnl_t7w);
          lt_hnl_t7sw = std::sin(lt_hnl_t7w);
          lt_hnl_t7al = lt_hnl_t7sw / (2.0 * lt_hnl_t7q);
          lt_hnl_t7ia = 1.0 / (1.0 + lt_hnl_t7al);
          lt_hnl_b0 = (1.0 - lt_hnl_t7cw) * 0.5 * lt_hnl_t7ia;
          lt_hnl_b1 = (1.0 - lt_hnl_t7cw) * lt_hnl_t7ia;
          lt_hnl_b2 = lt_hnl_b0;
          lt_hnl_a1 = -2.0 * lt_hnl_t7cw * lt_hnl_t7ia;
          lt_hnl_a2 = (1.0 - lt_hnl_t7al) * lt_hnl_t7ia;
          lt_hnl_z1 = 0.0;
          lt_hnl_z2 = 0.0;
          lt_fsh_t8c = gclamp(700.0 * lt_rt, 10.0, fs * 0.45);
          lt_fsh_t8q = gmax(0.05, 0.70710678);
          lt_fsh_t8w = 6.283185307179586 * lt_fsh_t8c / fs;
          lt_fsh_t8cw = std::cos(lt_fsh_t8w);
          lt_fsh_t8sw = std::sin(lt_fsh_t8w);
          lt_fsh_t8al = lt_fsh_t8sw / (2.0 * lt_fsh_t8q);
          lt_fsh_t8ia = 1.0 / (1.0 + lt_fsh_t8al);
          lt_fsh_b0 = (1.0 + lt_fsh_t8cw) * 0.5 * lt_fsh_t8ia;
          lt_fsh_b1 = -(1.0 + lt_fsh_t8cw) * lt_fsh_t8ia;
          lt_fsh_b2 = lt_fsh_b0;
          lt_fsh_a1 = -2.0 * lt_fsh_t8cw * lt_fsh_t8ia;
          lt_fsh_a2 = (1.0 - lt_fsh_t8al) * lt_fsh_t8ia;
          lt_fsh_z1 = 0.0;
          lt_fsh_z2 = 0.0;
          lt_fsl_t9c = gclamp(1100.0 * lt_rt, 10.0, fs * 0.45);
          lt_fsl_t9q = gmax(0.05, 0.70710678);
          lt_fsl_t9w = 6.283185307179586 * lt_fsl_t9c / fs;
          lt_fsl_t9cw = std::cos(lt_fsl_t9w);
          lt_fsl_t9sw = std::sin(lt_fsl_t9w);
          lt_fsl_t9al = lt_fsl_t9sw / (2.0 * lt_fsl_t9q);
          lt_fsl_t9ia = 1.0 / (1.0 + lt_fsl_t9al);
          lt_fsl_b0 = (1.0 - lt_fsl_t9cw) * 0.5 * lt_fsl_t9ia;
          lt_fsl_b1 = (1.0 - lt_fsl_t9cw) * lt_fsl_t9ia;
          lt_fsl_b2 = lt_fsl_b0;
          lt_fsl_a1 = -2.0 * lt_fsl_t9cw * lt_fsl_t9ia;
          lt_fsl_a2 = (1.0 - lt_fsl_t9al) * lt_fsl_t9ia;
          lt_fsl_z1 = 0.0;
          lt_fsl_z2 = 0.0;
          lt_tnh_t10c = gclamp(300.0 * lt_rt, 10.0, fs * 0.45);
          lt_tnh_t10q = gmax(0.05, 0.70710678);
          lt_tnh_t10w = 6.283185307179586 * lt_tnh_t10c / fs;
          lt_tnh_t10cw = std::cos(lt_tnh_t10w);
          lt_tnh_t10sw = std::sin(lt_tnh_t10w);
          lt_tnh_t10al = lt_tnh_t10sw / (2.0 * lt_tnh_t10q);
          lt_tnh_t10ia = 1.0 / (1.0 + lt_tnh_t10al);
          lt_tnh_b0 = (1.0 + lt_tnh_t10cw) * 0.5 * lt_tnh_t10ia;
          lt_tnh_b1 = -(1.0 + lt_tnh_t10cw) * lt_tnh_t10ia;
          lt_tnh_b2 = lt_tnh_b0;
          lt_tnh_a1 = -2.0 * lt_tnh_t10cw * lt_tnh_t10ia;
          lt_tnh_a2 = (1.0 - lt_tnh_t10al) * lt_tnh_t10ia;
          lt_tnh_z1 = 0.0;
          lt_tnh_z2 = 0.0;
          lt_tnl_t11c = gclamp(1500.0 * lt_rt, 10.0, fs * 0.45);
          lt_tnl_t11q = gmax(0.05, 0.70710678);
          lt_tnl_t11w = 6.283185307179586 * lt_tnl_t11c / fs;
          lt_tnl_t11cw = std::cos(lt_tnl_t11w);
          lt_tnl_t11sw = std::sin(lt_tnl_t11w);
          lt_tnl_t11al = lt_tnl_t11sw / (2.0 * lt_tnl_t11q);
          lt_tnl_t11ia = 1.0 / (1.0 + lt_tnl_t11al);
          lt_tnl_b0 = (1.0 - lt_tnl_t11cw) * 0.5 * lt_tnl_t11ia;
          lt_tnl_b1 = (1.0 - lt_tnl_t11cw) * lt_tnl_t11ia;
          lt_tnl_b2 = lt_tnl_b0;
          lt_tnl_a1 = -2.0 * lt_tnl_t11cw * lt_tnl_t11ia;
          lt_tnl_a2 = (1.0 - lt_tnl_t11al) * lt_tnl_t11ia;
          lt_tnl_z1 = 0.0;
          lt_tnl_z2 = 0.0;
          lt_active = 1.0;
        }
        if (lt_active > 0.0) {
          lt_mph = wrap2pi(lt_mph + 6.283185307179586 * (lt_mhz + lt_glide) / fs);
          lt_sph = wrap2pi(lt_sph + 6.283185307179586 * lt_shz / fs);
          lt_br = lt_br + (1.0 - lt_br) * lt_brc;
          lt_sr = lt_sr + (1.0 - lt_sr) * lt_src;
          lt_snr = lt_snr + (1.0 - lt_snr) * lt_snrc;
          lt_nr = lt_nr + (1.0 - lt_nr) * lt_nrc;
          lt_main = (std::sin(lt_mph) + 0.0095 * std::sin(2.0 * lt_mph) + 0.0032 * std::sin(3.0 * lt_mph)) * lt_me;
          lt_lower = 0.0;
          lt_strike = 1.05 * std::sin(lt_sph) * lt_se * lt_sr;
          lt_upper = 0.0;
          lt_ln = noise_();
          lt_ln1 = lt_lnh_b0 * lt_ln + lt_lnh_z1;
          lt_lnh_z1 = lt_lnh_b1 * lt_ln - lt_lnh_a1 * lt_ln1 + lt_lnh_z2;
          lt_lnh_z2 = lt_lnh_b2 * lt_ln - lt_lnh_a2 * lt_ln1;
          lt_ln2 = lt_lnl_b0 * lt_ln1 + lt_lnl_z1;
          lt_lnl_z1 = lt_lnl_b1 * lt_ln1 - lt_lnl_a1 * lt_ln2 + lt_lnl_z2;
          lt_lnl_z2 = lt_lnl_b2 * lt_ln1 - lt_lnl_a2 * lt_ln2;
          lt_hs = noise_();
          lt_hn1 = lt_hnh_b0 * lt_hs + lt_hnh_z1;
          lt_hnh_z1 = lt_hnh_b1 * lt_hs - lt_hnh_a1 * lt_hn1 + lt_hnh_z2;
          lt_hnh_z2 = lt_hnh_b2 * lt_hs - lt_hnh_a2 * lt_hn1;
          lt_hn2 = lt_hnl_b0 * lt_hn1 + lt_hnl_z1;
          lt_hnl_z1 = lt_hnl_b1 * lt_hn1 - lt_hnl_a1 * lt_hn2 + lt_hnl_z2;
          lt_hnl_z2 = lt_hnl_b2 * lt_hn1 - lt_hnl_a2 * lt_hn2;
          lt_tn = noise_();
          lt_tn1 = lt_tnh_b0 * lt_tn + lt_tnh_z1;
          lt_tnh_z1 = lt_tnh_b1 * lt_tn - lt_tnh_a1 * lt_tn1 + lt_tnh_z2;
          lt_tnh_z2 = lt_tnh_b2 * lt_tn - lt_tnh_a2 * lt_tn1;
          lt_tn2 = lt_tnl_b0 * lt_tn1 + lt_tnl_z1;
          lt_tnl_z1 = lt_tnl_b1 * lt_tn1 - lt_tnl_a1 * lt_tn2 + lt_tnl_z2;
          lt_tnl_z2 = lt_tnl_b2 * lt_tn1 - lt_tnl_a2 * lt_tn2;
          lt_snap = lt_snr * lt_sne;
          lt_burst = lt_nr * lt_bue;
          lt_exc = lt_ln2 * (0.5 * lt_snap + 0.03 * lt_burst) + lt_hn2 * (0.03 * lt_snap + 0.009 * lt_burst) + lt_tn2 * 0.01 * lt_nr * lt_tne;
          lt_fs1 = lt_fsh_b0 * lt_hs + lt_fsh_z1;
          lt_fsh_z1 = lt_fsh_b1 * lt_hs - lt_fsh_a1 * lt_fs1 + lt_fsh_z2;
          lt_fsh_z2 = lt_fsh_b2 * lt_hs - lt_fsh_a2 * lt_fs1;
          lt_fs2 = lt_fsl_b0 * lt_fs1 + lt_fsl_z1;
          lt_fsl_z1 = lt_fsl_b1 * lt_fs1 - lt_fsl_a1 * lt_fs2 + lt_fsl_z2;
          lt_fsl_z2 = lt_fsl_b2 * lt_fs1 - lt_fsl_a2 * lt_fs2;
          lt_exc = lt_exc + lt_fs2 * 1.1 * lt_snap;
          lt_out = ((lt_main + lt_lower + lt_upper) * lt_br + lt_strike + lt_exc) * 0.8;
          lt_me = lt_me * lt_mpole;
          lt_glide = lt_glide * lt_gpole;
          lt_le = lt_le * lt_lpole;
          lt_se = lt_se * lt_spole;
          lt_sne = lt_sne * lt_snp;
          lt_bue = lt_bue * lt_bup;
          lt_tne = lt_tne * lt_tnp;
          if (lt_me <= 0.000005 && 0.0 * lt_le <= 0.000005) { lt_active = 0.0; }
        }
        // ---- ht ----
        ht_fire = 0.0;
        ht_vel = 0.0;
        if (tr_ht != ht_trlast) {
          ht_trlast = tr_ht;
          ht_vel = tr_ht - 128.0 * std::floor(tr_ht / 128.0);
          ht_fire = 1.0;
        }
        if (ht_sq > 0.0) { ht_vel = gmax(ht_vel, seq_vel); ht_fire = 1.0; }
        if (ht_fire > 0.0) {
          ht_vg = velgain(ht_vel, acc);
          ht_dec = gclamp(ht_decay * 0.01, 0.05, 1.0);
          ht_rt = gclamp(std::pow(2.0, ht_tune / 12.0), 0.25, 4.0);
          ht_mhz = 208.0 * ht_rt;
          ht_glide = 0.0 * ht_rt;
          ht_gpole = opcoef(fs, 0.05);
          ht_lhz = ht_mhz * 0.652;
          ht_shz = 285.0 * ht_rt;
          ht_mph = 2.674;
          ht_lph = 1.059;
          ht_sph = 0.42;
          ht_me = 1.0;
          ht_le = 1.0;
          ht_se = 1.0;
          ht_uhz0 = ht_mhz * 1.467307692308;
          ht_uph0 = -2.987;
          ht_ue0 = 1.0;
          ht_up0 = t60coef(fs, 0.8 * ht_dec);
          ht_uhz1 = ht_mhz * 1.662019230769;
          ht_uph1 = 0.618;
          ht_ue1 = 1.0;
          ht_up1 = t60coef(fs, 0.8 * ht_dec);
          ht_uhz2 = ht_mhz * 1.764423076923;
          ht_uph2 = 1.002;
          ht_ue2 = 1.0;
          ht_up2 = t60coef(fs, 0.4 * ht_dec);
          ht_uhz3 = ht_mhz * 1.859134615385;
          ht_uph3 = 0.597;
          ht_ue3 = 1.0;
          ht_up3 = t60coef(fs, 0.36 * ht_dec);
          ht_sne = 1.0;
          ht_bue = 1.0;
          ht_tne = 1.0;
          ht_mpole = t60coef(fs, 0.218 * ht_dec);
          ht_lpole = t60coef(fs, 0.315 * ht_dec);
          ht_spole = t60coef(fs, 0.0125 * ht_dec);
          ht_snp = t60coef(fs, 0.012 * ht_dec);
          ht_bup = t60coef(fs, 0.08 * ht_dec);
          ht_tnp = t60coef(fs, 0.3 * ht_dec);
          ht_br = 0.0;
          ht_brc = 1.0 - opcoef(fs, 0.00012);
          ht_sr = 0.0;
          ht_src = 1.0 - opcoef(fs, 0.00003);
          ht_snr = 0.0;
          ht_snrc = 1.0 - opcoef(fs, 0.0004);
          ht_nr = 0.0;
          ht_nrc = 1.0 - opcoef(fs, 0.0015);
          ht_lnh_t12c = gclamp(300.0 * ht_rt, 10.0, fs * 0.45);
          ht_lnh_t12q = gmax(0.05, 0.70710678);
          ht_lnh_t12w = 6.283185307179586 * ht_lnh_t12c / fs;
          ht_lnh_t12cw = std::cos(ht_lnh_t12w);
          ht_lnh_t12sw = std::sin(ht_lnh_t12w);
          ht_lnh_t12al = ht_lnh_t12sw / (2.0 * ht_lnh_t12q);
          ht_lnh_t12ia = 1.0 / (1.0 + ht_lnh_t12al);
          ht_lnh_b0 = (1.0 + ht_lnh_t12cw) * 0.5 * ht_lnh_t12ia;
          ht_lnh_b1 = -(1.0 + ht_lnh_t12cw) * ht_lnh_t12ia;
          ht_lnh_b2 = ht_lnh_b0;
          ht_lnh_a1 = -2.0 * ht_lnh_t12cw * ht_lnh_t12ia;
          ht_lnh_a2 = (1.0 - ht_lnh_t12al) * ht_lnh_t12ia;
          ht_lnh_z1 = 0.0;
          ht_lnh_z2 = 0.0;
          ht_lnl_t13c = gclamp(3000.0 * ht_rt, 10.0, fs * 0.45);
          ht_lnl_t13q = gmax(0.05, 0.70710678);
          ht_lnl_t13w = 6.283185307179586 * ht_lnl_t13c / fs;
          ht_lnl_t13cw = std::cos(ht_lnl_t13w);
          ht_lnl_t13sw = std::sin(ht_lnl_t13w);
          ht_lnl_t13al = ht_lnl_t13sw / (2.0 * ht_lnl_t13q);
          ht_lnl_t13ia = 1.0 / (1.0 + ht_lnl_t13al);
          ht_lnl_b0 = (1.0 - ht_lnl_t13cw) * 0.5 * ht_lnl_t13ia;
          ht_lnl_b1 = (1.0 - ht_lnl_t13cw) * ht_lnl_t13ia;
          ht_lnl_b2 = ht_lnl_b0;
          ht_lnl_a1 = -2.0 * ht_lnl_t13cw * ht_lnl_t13ia;
          ht_lnl_a2 = (1.0 - ht_lnl_t13al) * ht_lnl_t13ia;
          ht_lnl_z1 = 0.0;
          ht_lnl_z2 = 0.0;
          ht_hnh_t14c = gclamp(700.0 * ht_rt, 10.0, fs * 0.45);
          ht_hnh_t14q = gmax(0.05, 0.70710678);
          ht_hnh_t14w = 6.283185307179586 * ht_hnh_t14c / fs;
          ht_hnh_t14cw = std::cos(ht_hnh_t14w);
          ht_hnh_t14sw = std::sin(ht_hnh_t14w);
          ht_hnh_t14al = ht_hnh_t14sw / (2.0 * ht_hnh_t14q);
          ht_hnh_t14ia = 1.0 / (1.0 + ht_hnh_t14al);
          ht_hnh_b0 = (1.0 + ht_hnh_t14cw) * 0.5 * ht_hnh_t14ia;
          ht_hnh_b1 = -(1.0 + ht_hnh_t14cw) * ht_hnh_t14ia;
          ht_hnh_b2 = ht_hnh_b0;
          ht_hnh_a1 = -2.0 * ht_hnh_t14cw * ht_hnh_t14ia;
          ht_hnh_a2 = (1.0 - ht_hnh_t14al) * ht_hnh_t14ia;
          ht_hnh_z1 = 0.0;
          ht_hnh_z2 = 0.0;
          ht_hnl_t15c = gclamp(12000.0 * ht_rt, 10.0, fs * 0.45);
          ht_hnl_t15q = gmax(0.05, 0.70710678);
          ht_hnl_t15w = 6.283185307179586 * ht_hnl_t15c / fs;
          ht_hnl_t15cw = std::cos(ht_hnl_t15w);
          ht_hnl_t15sw = std::sin(ht_hnl_t15w);
          ht_hnl_t15al = ht_hnl_t15sw / (2.0 * ht_hnl_t15q);
          ht_hnl_t15ia = 1.0 / (1.0 + ht_hnl_t15al);
          ht_hnl_b0 = (1.0 - ht_hnl_t15cw) * 0.5 * ht_hnl_t15ia;
          ht_hnl_b1 = (1.0 - ht_hnl_t15cw) * ht_hnl_t15ia;
          ht_hnl_b2 = ht_hnl_b0;
          ht_hnl_a1 = -2.0 * ht_hnl_t15cw * ht_hnl_t15ia;
          ht_hnl_a2 = (1.0 - ht_hnl_t15al) * ht_hnl_t15ia;
          ht_hnl_z1 = 0.0;
          ht_hnl_z2 = 0.0;
          ht_tnh_t16c = gclamp(300.0 * ht_rt, 10.0, fs * 0.45);
          ht_tnh_t16q = gmax(0.05, 0.70710678);
          ht_tnh_t16w = 6.283185307179586 * ht_tnh_t16c / fs;
          ht_tnh_t16cw = std::cos(ht_tnh_t16w);
          ht_tnh_t16sw = std::sin(ht_tnh_t16w);
          ht_tnh_t16al = ht_tnh_t16sw / (2.0 * ht_tnh_t16q);
          ht_tnh_t16ia = 1.0 / (1.0 + ht_tnh_t16al);
          ht_tnh_b0 = (1.0 + ht_tnh_t16cw) * 0.5 * ht_tnh_t16ia;
          ht_tnh_b1 = -(1.0 + ht_tnh_t16cw) * ht_tnh_t16ia;
          ht_tnh_b2 = ht_tnh_b0;
          ht_tnh_a1 = -2.0 * ht_tnh_t16cw * ht_tnh_t16ia;
          ht_tnh_a2 = (1.0 - ht_tnh_t16al) * ht_tnh_t16ia;
          ht_tnh_z1 = 0.0;
          ht_tnh_z2 = 0.0;
          ht_tnl_t17c = gclamp(1500.0 * ht_rt, 10.0, fs * 0.45);
          ht_tnl_t17q = gmax(0.05, 0.70710678);
          ht_tnl_t17w = 6.283185307179586 * ht_tnl_t17c / fs;
          ht_tnl_t17cw = std::cos(ht_tnl_t17w);
          ht_tnl_t17sw = std::sin(ht_tnl_t17w);
          ht_tnl_t17al = ht_tnl_t17sw / (2.0 * ht_tnl_t17q);
          ht_tnl_t17ia = 1.0 / (1.0 + ht_tnl_t17al);
          ht_tnl_b0 = (1.0 - ht_tnl_t17cw) * 0.5 * ht_tnl_t17ia;
          ht_tnl_b1 = (1.0 - ht_tnl_t17cw) * ht_tnl_t17ia;
          ht_tnl_b2 = ht_tnl_b0;
          ht_tnl_a1 = -2.0 * ht_tnl_t17cw * ht_tnl_t17ia;
          ht_tnl_a2 = (1.0 - ht_tnl_t17al) * ht_tnl_t17ia;
          ht_tnl_z1 = 0.0;
          ht_tnl_z2 = 0.0;
          ht_active = 1.0;
        }
        if (ht_active > 0.0) {
          ht_mph = wrap2pi(ht_mph + 6.283185307179586 * (ht_mhz + ht_glide) / fs);
          ht_lph = wrap2pi(ht_lph + 6.283185307179586 * ht_lhz / fs);
          ht_sph = wrap2pi(ht_sph + 6.283185307179586 * ht_shz / fs);
          ht_uph0 = wrap2pi(ht_uph0 + 6.283185307179586 * ht_uhz0 / fs);
          ht_uph1 = wrap2pi(ht_uph1 + 6.283185307179586 * ht_uhz1 / fs);
          ht_uph2 = wrap2pi(ht_uph2 + 6.283185307179586 * ht_uhz2 / fs);
          ht_uph3 = wrap2pi(ht_uph3 + 6.283185307179586 * ht_uhz3 / fs);
          ht_br = ht_br + (1.0 - ht_br) * ht_brc;
          ht_sr = ht_sr + (1.0 - ht_sr) * ht_src;
          ht_snr = ht_snr + (1.0 - ht_snr) * ht_snrc;
          ht_nr = ht_nr + (1.0 - ht_nr) * ht_nrc;
          ht_main = (std::sin(ht_mph) + 0.0095 * std::sin(2.0 * ht_mph) + 0.0032 * std::sin(3.0 * ht_mph)) * ht_me;
          ht_lower = 0.09 * std::sin(ht_lph) * ht_le;
          ht_strike = 0.71 * std::sin(ht_sph) * ht_se * ht_sr;
          ht_upper = 0.002617 * std::sin(ht_uph0) * ht_ue0 + 0.001954 * std::sin(ht_uph1) * ht_ue1 + 0.003381 * std::sin(ht_uph2) * ht_ue2 + 0.003546 * std::sin(ht_uph3) * ht_ue3;
          ht_ln = noise_();
          ht_ln1 = ht_lnh_b0 * ht_ln + ht_lnh_z1;
          ht_lnh_z1 = ht_lnh_b1 * ht_ln - ht_lnh_a1 * ht_ln1 + ht_lnh_z2;
          ht_lnh_z2 = ht_lnh_b2 * ht_ln - ht_lnh_a2 * ht_ln1;
          ht_ln2 = ht_lnl_b0 * ht_ln1 + ht_lnl_z1;
          ht_lnl_z1 = ht_lnl_b1 * ht_ln1 - ht_lnl_a1 * ht_ln2 + ht_lnl_z2;
          ht_lnl_z2 = ht_lnl_b2 * ht_ln1 - ht_lnl_a2 * ht_ln2;
          ht_hs = noise_();
          ht_hn1 = ht_hnh_b0 * ht_hs + ht_hnh_z1;
          ht_hnh_z1 = ht_hnh_b1 * ht_hs - ht_hnh_a1 * ht_hn1 + ht_hnh_z2;
          ht_hnh_z2 = ht_hnh_b2 * ht_hs - ht_hnh_a2 * ht_hn1;
          ht_hn2 = ht_hnl_b0 * ht_hn1 + ht_hnl_z1;
          ht_hnl_z1 = ht_hnl_b1 * ht_hn1 - ht_hnl_a1 * ht_hn2 + ht_hnl_z2;
          ht_hnl_z2 = ht_hnl_b2 * ht_hn1 - ht_hnl_a2 * ht_hn2;
          ht_tn = noise_();
          ht_tn1 = ht_tnh_b0 * ht_tn + ht_tnh_z1;
          ht_tnh_z1 = ht_tnh_b1 * ht_tn - ht_tnh_a1 * ht_tn1 + ht_tnh_z2;
          ht_tnh_z2 = ht_tnh_b2 * ht_tn - ht_tnh_a2 * ht_tn1;
          ht_tn2 = ht_tnl_b0 * ht_tn1 + ht_tnl_z1;
          ht_tnl_z1 = ht_tnl_b1 * ht_tn1 - ht_tnl_a1 * ht_tn2 + ht_tnl_z2;
          ht_tnl_z2 = ht_tnl_b2 * ht_tn1 - ht_tnl_a2 * ht_tn2;
          ht_snap = ht_snr * ht_sne;
          ht_burst = ht_nr * ht_bue;
          ht_exc = ht_ln2 * (0.5 * ht_snap + 0.03 * ht_burst) + ht_hn2 * (0.03 * ht_snap + 0.009 * ht_burst) + ht_tn2 * 0.01 * ht_nr * ht_tne;
          ht_out = ((ht_main + ht_lower + ht_upper) * ht_br + ht_strike + ht_exc) * 0.8;
          ht_me = ht_me * ht_mpole;
          ht_glide = ht_glide * ht_gpole;
          ht_le = ht_le * ht_lpole;
          ht_se = ht_se * ht_spole;
          ht_ue0 = ht_ue0 * ht_up0;
          ht_ue1 = ht_ue1 * ht_up1;
          ht_ue2 = ht_ue2 * ht_up2;
          ht_ue3 = ht_ue3 * ht_up3;
          ht_sne = ht_sne * ht_snp;
          ht_bue = ht_bue * ht_bup;
          ht_tne = ht_tne * ht_tnp;
          if (ht_me <= 0.000005 && 0.09 * ht_le <= 0.000005 && 0.002617 * ht_ue0 <= 0.000005 && 0.001954 * ht_ue1 <= 0.000005 && 0.003381 * ht_ue2 <= 0.000005 && 0.003546 * ht_ue3 <= 0.000005) { ht_active = 0.0; }
        }
        // ---- cp ----
        cp_fire = 0.0;
        cp_vel = 0.0;
        if (tr_cp != cp_trlast) {
          cp_trlast = tr_cp;
          cp_vel = tr_cp - 128.0 * std::floor(tr_cp / 128.0);
          cp_fire = 1.0;
        }
        if (cp_sq > 0.0) { cp_vel = gmax(cp_vel, seq_vel); cp_fire = 1.0; }
        if (cp_fire > 0.0) {
          cp_vg = velgain(cp_vel, acc);
          cp_dec = gclamp(cp_decay * 0.01, 0.05, 1.0);
          cp_rt = gclamp(std::pow(2.0, cp_tune / 12.0), 0.5, 2.0);
          cp_nn = gclamp(cp_noise * 0.01, 0.0, 1.0);
          cp_nlg = 2.0 * gmin(cp_nn, 0.5);
          cp_air = 0.0;
          if (cp_nn > 0.5) { cp_air = 2.0 * (cp_nn - 0.5); }
          cp_stp = 44100.0 * cp_rt / fs;
          cp_tapn = gmax(1.0, gmin(std::ceil(192.0 / cp_stp), 384.0));
          for (k = 0.0; k < 4.0; k = k + 1.0) {
            cp_se = 0.0;
            for (j = 0.0; j < 192.0; j = j + 1.0) { cp_sv = peek_(tbl, k * 192.0 + j); cp_se = cp_se + cp_sv * cp_sv; }
            cp_re = 0.0;
            for (j = 0.0; j < cp_tapn; j = j + 1.0) {
              cp_ps = j * cp_stp;
              cp_ii = std::floor(cp_ps);
              cp_fr = cp_ps - cp_ii;
              cp_p0 = 0.0; cp_p1 = 0.0; cp_p2 = 0.0; cp_p3 = 0.0;
              if (cp_ii - 1.0 >= 0.0 && cp_ii - 1.0 < 192.0) { cp_p0 = peek_(tbl, k * 192.0 + cp_ii - 1.0); }
              if (cp_ii >= 0.0 && cp_ii < 192.0) { cp_p1 = peek_(tbl, k * 192.0 + cp_ii); }
              if (cp_ii + 1.0 >= 0.0 && cp_ii + 1.0 < 192.0) { cp_p2 = peek_(tbl, k * 192.0 + cp_ii + 1.0); }
              if (cp_ii + 2.0 >= 0.0 && cp_ii + 2.0 < 192.0) { cp_p3 = peek_(tbl, k * 192.0 + cp_ii + 2.0); }
              cp_f2 = cp_fr * cp_fr;
              cp_f3 = cp_f2 * cp_fr;
              cp_cv = 0.5 * (2.0 * cp_p1 + (-cp_p0 + cp_p2) * cp_fr + (2.0 * cp_p0 - 5.0 * cp_p1 + 4.0 * cp_p2 - cp_p3) * cp_f2 + (-cp_p0 + 3.0 * cp_p1 - 3.0 * cp_p2 + cp_p3) * cp_f3);
              poke_(cp_c, cp_cv, j * 4.0 + k);
              cp_re = cp_re + cp_cv * cp_cv;
            }
            cp_sc = std::sqrt(cp_se / gmax(cp_re, 0.00000000000000000001));
            for (j = 0.0; j < cp_tapn; j = j + 1.0) { poke_(cp_c, peek_(cp_c, j * 4.0 + k) * cp_sc, j * 4.0 + k); }
          }
          for (j = 0.0; j < cp_tapn; j = j + 1.0) {
            cp_g = (noise_() + noise_() + noise_() + noise_() + noise_() + noise_()) * 0.70710678118654752;
            poke_(cp_ring, cp_g, cp_pos);
            poke_(cp_ring, cp_g, cp_pos + 512.0);
            cp_pos = cp_pos + 1.0;
            if (cp_pos >= 512.0) { cp_pos = 0.0; }
          }
          cp_blf = 0.425 * 44100.0 * cp_rt;
          cp_bl = 0.0;
          if (cp_blf < 0.45 * fs) { cp_bl = 1.0; }
          cp_la1_t18c = gclamp(cp_blf, 10.0, fs * 0.45);
          cp_la1_t18q = gmax(0.05, 0.5411961);
          cp_la1_t18w = 6.283185307179586 * cp_la1_t18c / fs;
          cp_la1_t18cw = std::cos(cp_la1_t18w);
          cp_la1_t18sw = std::sin(cp_la1_t18w);
          cp_la1_t18al = cp_la1_t18sw / (2.0 * cp_la1_t18q);
          cp_la1_t18ia = 1.0 / (1.0 + cp_la1_t18al);
          cp_la1_b0 = (1.0 - cp_la1_t18cw) * 0.5 * cp_la1_t18ia;
          cp_la1_b1 = (1.0 - cp_la1_t18cw) * cp_la1_t18ia;
          cp_la1_b2 = cp_la1_b0;
          cp_la1_a1 = -2.0 * cp_la1_t18cw * cp_la1_t18ia;
          cp_la1_a2 = (1.0 - cp_la1_t18al) * cp_la1_t18ia;
          cp_la1_z1 = 0.0;
          cp_la1_z2 = 0.0;
          cp_la2_t19c = gclamp(cp_blf, 10.0, fs * 0.45);
          cp_la2_t19q = gmax(0.05, 1.3065630);
          cp_la2_t19w = 6.283185307179586 * cp_la2_t19c / fs;
          cp_la2_t19cw = std::cos(cp_la2_t19w);
          cp_la2_t19sw = std::sin(cp_la2_t19w);
          cp_la2_t19al = cp_la2_t19sw / (2.0 * cp_la2_t19q);
          cp_la2_t19ia = 1.0 / (1.0 + cp_la2_t19al);
          cp_la2_b0 = (1.0 - cp_la2_t19cw) * 0.5 * cp_la2_t19ia;
          cp_la2_b1 = (1.0 - cp_la2_t19cw) * cp_la2_t19ia;
          cp_la2_b2 = cp_la2_b0;
          cp_la2_a1 = -2.0 * cp_la2_t19cw * cp_la2_t19ia;
          cp_la2_a2 = (1.0 - cp_la2_t19al) * cp_la2_t19ia;
          cp_la2_z1 = 0.0;
          cp_la2_z2 = 0.0;
          cp_lb1_t20c = gclamp(cp_blf, 10.0, fs * 0.45);
          cp_lb1_t20q = gmax(0.05, 0.5411961);
          cp_lb1_t20w = 6.283185307179586 * cp_lb1_t20c / fs;
          cp_lb1_t20cw = std::cos(cp_lb1_t20w);
          cp_lb1_t20sw = std::sin(cp_lb1_t20w);
          cp_lb1_t20al = cp_lb1_t20sw / (2.0 * cp_lb1_t20q);
          cp_lb1_t20ia = 1.0 / (1.0 + cp_lb1_t20al);
          cp_lb1_b0 = (1.0 - cp_lb1_t20cw) * 0.5 * cp_lb1_t20ia;
          cp_lb1_b1 = (1.0 - cp_lb1_t20cw) * cp_lb1_t20ia;
          cp_lb1_b2 = cp_lb1_b0;
          cp_lb1_a1 = -2.0 * cp_lb1_t20cw * cp_lb1_t20ia;
          cp_lb1_a2 = (1.0 - cp_lb1_t20al) * cp_lb1_t20ia;
          cp_lb1_z1 = 0.0;
          cp_lb1_z2 = 0.0;
          cp_lb2_t21c = gclamp(cp_blf, 10.0, fs * 0.45);
          cp_lb2_t21q = gmax(0.05, 1.3065630);
          cp_lb2_t21w = 6.283185307179586 * cp_lb2_t21c / fs;
          cp_lb2_t21cw = std::cos(cp_lb2_t21w);
          cp_lb2_t21sw = std::sin(cp_lb2_t21w);
          cp_lb2_t21al = cp_lb2_t21sw / (2.0 * cp_lb2_t21q);
          cp_lb2_t21ia = 1.0 / (1.0 + cp_lb2_t21al);
          cp_lb2_b0 = (1.0 - cp_lb2_t21cw) * 0.5 * cp_lb2_t21ia;
          cp_lb2_b1 = (1.0 - cp_lb2_t21cw) * cp_lb2_t21ia;
          cp_lb2_b2 = cp_lb2_b0;
          cp_lb2_a1 = -2.0 * cp_lb2_t21cw * cp_lb2_t21ia;
          cp_lb2_a2 = (1.0 - cp_lb2_t21al) * cp_lb2_t21ia;
          cp_lb2_z1 = 0.0;
          cp_lb2_z2 = 0.0;
          cp_ahc = std::exp(-6.283185307179586 * gmin(3000.0, 0.35 * fs) / fs);
          cp_ahi = 0.0;
          cp_aho = 0.0;
          cp_maxs = gmax(1.0, std::ceil(fs * 0.301));
          cp_si = 0.0;
          cp_active = 1.0;
        }
        if (cp_active > 0.0) {
          cp_g = (noise_() + noise_() + noise_() + noise_() + noise_() + noise_()) * 0.70710678118654752;
          poke_(cp_ring, cp_g, cp_pos);
          poke_(cp_ring, cp_g, cp_pos + 512.0);
          cp_pos = cp_pos + 1.0;
          if (cp_pos >= 512.0) { cp_pos = 0.0; }
          cp_c0 = 0.0; cp_c1 = 0.0; cp_c2 = 0.0; cp_c3 = 0.0;
          cp_base = cp_pos + 511.0;
          for (j = 0.0; j < cp_tapn; j = j + 1.0) {
            cp_s = peek_(cp_ring, cp_base - j);
            cp_ci = j * 4.0;
            cp_c0 = cp_c0 + peek_(cp_c, cp_ci) * cp_s;
            cp_c1 = cp_c1 + peek_(cp_c, cp_ci + 1.0) * cp_s;
            cp_c2 = cp_c2 + peek_(cp_c, cp_ci + 2.0) * cp_s;
            cp_c3 = cp_c3 + peek_(cp_c, cp_ci + 3.0) * cp_s;
          }
          cp_tm = cp_si / fs;
          cp_dds = 1.0 + 0.45 * cp_air;
          cp_short = 2.0 * rsg(cp_tm, 0.0, 0.0, 0.0003, 0.0125 * cp_dec) + 1.85 * rsg(cp_tm, 0.010737, 0.00016, 0.0003, 0.014 * cp_dec) + 1.16 * rsg(cp_tm, 0.021735, 0.00016, 0.0003, 0.019 * cp_dec) + 0.46 * rsg(cp_tm, 0.032778, 0.00016, 0.0003, 0.025 * cp_dec);
          cp_dry = cp_c0 * cp_short;
          cp_full = cp_dry;
          cp_ta = cp_tm - 0.03278;
          if (cp_ta >= 0.0) {
            cp_tat = 1.0;
            if (cp_ta < 0.0002) { cp_sn = std::sin(0.5 * 3.141592653589793 * cp_ta / 0.0002); cp_tat = cp_sn * cp_sn; }
            cp_tda = gmax(cp_ta - 0.0002 - 0.019, 0.0);
            cp_tenv = 0.72 * cp_tat * (0.955 * std::exp(-6.9077553 * cp_tda / ((0.225 * cp_dec) * cp_dds)) + 0.045 * std::exp(-6.9077553 * cp_tda / ((0.89 * cp_dec) * cp_dds)));
            cp_fm = rcr(cp_tm, 0.055, 0.08);
            cp_tc = (1.0 - cp_fm) * cp_c1 + cp_fm * cp_c2;
            cp_lm = rcr(cp_tm, 0.11, 0.17);
            cp_tc = (1.0 - cp_lm) * cp_tc + cp_lm * cp_c3;
            cp_full = cp_full + cp_tenv * cp_tc;
          }
          cp_snp = 0.55 * cp_c0 * rsg(cp_tm, 0.032778, 0.00012, 0.00035, 0.025 * cp_dec);
          cp_full = cp_full + cp_snp;
          cp_dry = cp_dry + cp_snp;
          cp_full = cp_full + 0.022 * cp_dec * cp_c3 * rsg(cp_tm, 0.165, 0.02, 0.0, (0.76 * cp_dec) * cp_dds);
          cp_full = cp_full + 0.004 * cp_dec * cp_c3 * rsg(cp_tm, 0.25, 0.015, 0.0, (0.76 * cp_dec) * cp_dds);
          if (cp_bl > 0.0) {
            cp_fa = cp_la1_b0 * cp_full + cp_la1_z1;
            cp_la1_z1 = cp_la1_b1 * cp_full - cp_la1_a1 * cp_fa + cp_la1_z2;
            cp_la1_z2 = cp_la1_b2 * cp_full - cp_la1_a2 * cp_fa;
            cp_fb2 = cp_la2_b0 * cp_fa + cp_la2_z1;
            cp_la2_z1 = cp_la2_b1 * cp_fa - cp_la2_a1 * cp_fb2 + cp_la2_z2;
            cp_la2_z2 = cp_la2_b2 * cp_fa - cp_la2_a2 * cp_fb2;
            cp_da = cp_lb1_b0 * cp_dry + cp_lb1_z1;
            cp_lb1_z1 = cp_lb1_b1 * cp_dry - cp_lb1_a1 * cp_da + cp_lb1_z2;
            cp_lb1_z2 = cp_lb1_b2 * cp_dry - cp_lb1_a2 * cp_da;
            cp_db2 = cp_lb2_b0 * cp_da + cp_lb2_z1;
            cp_lb2_z1 = cp_lb2_b1 * cp_da - cp_lb2_a1 * cp_db2 + cp_lb2_z2;
            cp_lb2_z2 = cp_lb2_b2 * cp_da - cp_lb2_a2 * cp_db2;
            cp_full = cp_fb2;
            cp_dry = cp_db2;
          }
          cp_fp = gclamp((cp_tm - 0.29) / (0.301 - 0.29), 0.0, 1.0);
          cp_fb = 0.5 + 0.5 * std::cos(3.141592653589793 * cp_fp);
          cp_fade = cp_fb * cp_fb * cp_fb * cp_fb;
          cp_cal = 3.0 * std::tanh(cp_full * cp_fade / 3.0);
          cp_sat = cp_cal;
          if (cp_air > 0.0) {
            cp_dr = 3.0 * std::tanh(cp_dry * cp_fade / 3.0);
            cp_cn = cp_cal - cp_dr;
            cp_aho = cp_ahc * (cp_aho + cp_cn - cp_ahi);
            cp_ahi = cp_cn;
            cp_lw = rcr(cp_tm, 0.045, 0.17);
            cp_dg = 1.0 + cp_air * (0.4 + 0.6 * cp_lw);
            cp_dm = cp_air * (0.2 + 0.55 * cp_lw);
            cp_dn = 0.75 * std::tanh(2.0 * cp_cn / 0.75);
            cp_am = cp_air * (0.15 + 0.5 * cp_lw);
            cp_sat = cp_dr + cp_dg * cp_cn + cp_dm * cp_dn + cp_am * cp_aho;
          }
          else if (cp_nlg != 1.0) {
            cp_dr2 = 3.0 * std::tanh(cp_dry * cp_fade / 3.0);
            cp_sat = cp_dr2 + cp_nlg * (cp_cal - cp_dr2);
          }
          cp_out = cp_sat * 0.1793169072680733;
          cp_si = cp_si + 1.0;
          if (cp_si >= cp_maxs) { cp_active = 0.0; }
        }
        // ---- ch ----
        ch_fire = 0.0;
        ch_vel = 0.0;
        if (tr_ch != ch_trlast) {
          ch_trlast = tr_ch;
          ch_vel = tr_ch - 128.0 * std::floor(tr_ch / 128.0);
          ch_fire = 1.0;
        }
        if (ch_sq > 0.0) { ch_vel = gmax(ch_vel, seq_vel); ch_fire = 1.0; }
        if (ch_fire > 0.0) {
          ch_vg = velgain(ch_vel, acc);
          ch_dec = gclamp(ch_decay * 0.01, 0.0, 1.0);
          ch_fr = gclamp(std::pow(2.0, ch_tune / 12.0), 0.0625, 16.0);
          for (i = 0.0; i < 47.0; i = i + 1.0) {
            ch_f = peek_(tbl, 768.0 + i) * ch_fr;
            ch_rr = ch_f / fs;
            ch_lv = 1.0;
            if (ch_rr >= 0.48) { ch_lv = 0.0; }
            else if (ch_rr > 0.4) { ch_tp = (0.48 - ch_rr) / 0.08; ch_lv = ch_tp * ch_tp * (3.0 - 2.0 * ch_tp); }
            poke_(ch_ph, (noise_() + 1.0) * 0.3 * 3.141592653589793, i);
            ch_ic = 0.0;
            if (ch_lv > 0.0) { ch_ic = 6.283185307179586 * ch_rr; }
            poke_(ch_inc, ch_ic, i);
            poke_(ch_amp, peek_(tbl, 815.0 + i) * ch_lv, i);
            poke_(ch_wob, 0.0, i);
          }
          ch_bae = 1.0;
          ch_bac = dcoef(fs, 0.04);
          ch_cke = 1.0;
          ch_ckc = dcoef(fs, 0.003);
          ch_hp_t22c = gclamp(6800.0 * ch_fr, 10.0, fs * 0.45);
          ch_hp_t22q = gmax(0.05, 0.7);
          ch_hp_t22w = 6.283185307179586 * ch_hp_t22c / fs;
          ch_hp_t22cw = std::cos(ch_hp_t22w);
          ch_hp_t22sw = std::sin(ch_hp_t22w);
          ch_hp_t22al = ch_hp_t22sw / (2.0 * ch_hp_t22q);
          ch_hp_t22ia = 1.0 / (1.0 + ch_hp_t22al);
          ch_hp_b0 = (1.0 + ch_hp_t22cw) * 0.5 * ch_hp_t22ia;
          ch_hp_b1 = -(1.0 + ch_hp_t22cw) * ch_hp_t22ia;
          ch_hp_b2 = ch_hp_b0;
          ch_hp_a1 = -2.0 * ch_hp_t22cw * ch_hp_t22ia;
          ch_hp_a2 = (1.0 - ch_hp_t22al) * ch_hp_t22ia;
          ch_hp_z1 = 0.0;
          ch_hp_z2 = 0.0;
          ch_lp_t23c = gclamp(12500.0 * ch_fr, 10.0, fs * 0.45);
          ch_lp_t23q = gmax(0.05, 0.707);
          ch_lp_t23w = 6.283185307179586 * ch_lp_t23c / fs;
          ch_lp_t23cw = std::cos(ch_lp_t23w);
          ch_lp_t23sw = std::sin(ch_lp_t23w);
          ch_lp_t23al = ch_lp_t23sw / (2.0 * ch_lp_t23q);
          ch_lp_t23ia = 1.0 / (1.0 + ch_lp_t23al);
          ch_lp_b0 = (1.0 - ch_lp_t23cw) * 0.5 * ch_lp_t23ia;
          ch_lp_b1 = (1.0 - ch_lp_t23cw) * ch_lp_t23ia;
          ch_lp_b2 = ch_lp_b0;
          ch_lp_a1 = -2.0 * ch_lp_t23cw * ch_lp_t23ia;
          ch_lp_a2 = (1.0 - ch_lp_t23al) * ch_lp_t23ia;
          ch_lp_z1 = 0.0;
          ch_lp_z2 = 0.0;
          ch_dcx = 0.0;
          ch_dcy = 0.0;
          ch_wa = 1.0 - dcoef(fs, 0.02);
          ch_wd = 0.0007 * std::sqrt(6.0 * ch_wa);
          ch_fdc = dcoef(fs, gmax(0.004, ch_dec * 0.03355));
          ch_sdc = dcoef(fs, gmax(0.004, ch_dec * 0.03355));
          ch_fe = 1.0;
          ch_se = 1.0;
          ch_ac = 1.0 - dcoef(fs, 0.0003);
          ch_ae = 0.0;
          ch_nd = gmax(0.028, ch_dec * 0.219954648526);
          ch_nf = gmax(1.0, std::floor(ch_nd * fs + 0.5));
          ch_gff = gmin(std::floor(0.045 * fs + 0.5), ch_nf);
          ch_igf = 0.0;
          if (ch_gff > 0.0) { ch_igf = 1.0 / ch_gff; }
          ch_fi = 0.0;
          ch_active = 1.0;
        }
        if (ch_active > 0.0) {
          if (ch_fi >= ch_nf) { ch_active = 0.0; }
          else {
          ch_bg = 1.0 + 0.7 * ch_bae;
          ch_bae = ch_bae * ch_bac;
          ch_ton = 0.0;
          for (i = 0.0; i < 47.0; i = i + 1.0) {
            ch_wv = peek_(ch_wob, i);
            ch_wv = ch_wv + ch_wd * noise_() - ch_wa * ch_wv;
            poke_(ch_wob, ch_wv, i);
            ch_pv = peek_(ch_ph, i);
            ch_lg = 1.0;
            if (peek_(tbl, 862.0 + i) > 0.0) { ch_lg = ch_bg; }
            ch_ton = ch_ton + std::sin(ch_pv) * peek_(ch_amp, i) * ch_lg;
            ch_pv = ch_pv + peek_(ch_inc, i) * (1.0 + ch_wv);
            if (ch_pv >= 6.283185307179586) { ch_pv = ch_pv - 6.283185307179586; }
            poke_(ch_ph, ch_pv, i);
          }
          ch_nx = noise_();
          ch_wn = ch_nx - ch_nin + 0.995 * ch_nout;
          ch_nin = ch_nx;
          ch_nout = ch_wn;
          ch_n1 = ch_hp_b0 * ch_wn + ch_hp_z1;
          ch_hp_z1 = ch_hp_b1 * ch_wn - ch_hp_a1 * ch_n1 + ch_hp_z2;
          ch_hp_z2 = ch_hp_b2 * ch_wn - ch_hp_a2 * ch_n1;
          ch_nz = ch_lp_b0 * ch_n1 + ch_lp_z1;
          ch_lp_z1 = ch_lp_b1 * ch_n1 - ch_lp_a1 * ch_nz + ch_lp_z2;
          ch_lp_z2 = ch_lp_b2 * ch_n1 - ch_lp_a2 * ch_nz;
          ch_ae = ch_ae + ch_ac * (1.0 - ch_ae);
          ch_fe = ch_fe * ch_fdc;
          ch_se = ch_se * ch_sdc;
          ch_env = ch_ae * (1.0 * ch_fe + 0.0 * ch_se);
          if (ch_gff > 0.0) { ch_left = ch_nf - ch_fi; if (ch_left < ch_gff) { ch_env = ch_env * ch_left * ch_igf; } }
          ch_src = std::tanh((ch_ton * 0.11 + ch_nz * 0.909) * 0.6);
          ch_bd = ch_src * ch_env + ch_nz * 0.35 * ch_cke;
          ch_cke = ch_cke * ch_ckc;
          ch_o = ch_bd * 1.75;
          ch_y = ch_o - ch_dcx + 0.995 * ch_dcy;
          ch_dcx = ch_o;
          ch_dcy = ch_y;
          ch_out = ch_y;
          ch_fi = ch_fi + 1.0;
          if (ch_fi >= ch_nf) { ch_active = 0.0; }
          }
        }
        // ---- oh ----
        oh_fire = 0.0;
        oh_vel = 0.0;
        if (tr_oh != oh_trlast) {
          oh_trlast = tr_oh;
          oh_vel = tr_oh - 128.0 * std::floor(tr_oh / 128.0);
          oh_fire = 1.0;
        }
        if (oh_sq > 0.0) { oh_vel = gmax(oh_vel, seq_vel); oh_fire = 1.0; }
        if (oh_fire > 0.0) {
          oh_vg = velgain(oh_vel, acc);
          oh_dec = gclamp(oh_decay * 0.01, 0.0, 1.0);
          oh_fr = gclamp(std::pow(2.0, oh_tune / 12.0), 0.0625, 16.0);
          for (i = 0.0; i < 47.0; i = i + 1.0) {
            oh_f = peek_(tbl, 768.0 + i) * oh_fr;
            oh_rr = oh_f / fs;
            oh_lv = 1.0;
            if (oh_rr >= 0.48) { oh_lv = 0.0; }
            else if (oh_rr > 0.4) { oh_tp = (0.48 - oh_rr) / 0.08; oh_lv = oh_tp * oh_tp * (3.0 - 2.0 * oh_tp); }
            poke_(oh_ph, (noise_() + 1.0) * 0.3 * 3.141592653589793, i);
            oh_ic = 0.0;
            if (oh_lv > 0.0) { oh_ic = 6.283185307179586 * oh_rr; }
            poke_(oh_inc, oh_ic, i);
            poke_(oh_amp, peek_(tbl, 815.0 + i) * oh_lv, i);
            poke_(oh_wob, 0.0, i);
          }
          oh_bae = 1.0;
          oh_bac = dcoef(fs, 0.04);
          oh_cke = 1.0;
          oh_ckc = dcoef(fs, 0.003);
          oh_hp_t24c = gclamp(6800.0 * oh_fr, 10.0, fs * 0.45);
          oh_hp_t24q = gmax(0.05, 0.7);
          oh_hp_t24w = 6.283185307179586 * oh_hp_t24c / fs;
          oh_hp_t24cw = std::cos(oh_hp_t24w);
          oh_hp_t24sw = std::sin(oh_hp_t24w);
          oh_hp_t24al = oh_hp_t24sw / (2.0 * oh_hp_t24q);
          oh_hp_t24ia = 1.0 / (1.0 + oh_hp_t24al);
          oh_hp_b0 = (1.0 + oh_hp_t24cw) * 0.5 * oh_hp_t24ia;
          oh_hp_b1 = -(1.0 + oh_hp_t24cw) * oh_hp_t24ia;
          oh_hp_b2 = oh_hp_b0;
          oh_hp_a1 = -2.0 * oh_hp_t24cw * oh_hp_t24ia;
          oh_hp_a2 = (1.0 - oh_hp_t24al) * oh_hp_t24ia;
          oh_hp_z1 = 0.0;
          oh_hp_z2 = 0.0;
          oh_lp_t25c = gclamp(16000.0 * oh_fr, 10.0, fs * 0.45);
          oh_lp_t25q = gmax(0.05, 0.707);
          oh_lp_t25w = 6.283185307179586 * oh_lp_t25c / fs;
          oh_lp_t25cw = std::cos(oh_lp_t25w);
          oh_lp_t25sw = std::sin(oh_lp_t25w);
          oh_lp_t25al = oh_lp_t25sw / (2.0 * oh_lp_t25q);
          oh_lp_t25ia = 1.0 / (1.0 + oh_lp_t25al);
          oh_lp_b0 = (1.0 - oh_lp_t25cw) * 0.5 * oh_lp_t25ia;
          oh_lp_b1 = (1.0 - oh_lp_t25cw) * oh_lp_t25ia;
          oh_lp_b2 = oh_lp_b0;
          oh_lp_a1 = -2.0 * oh_lp_t25cw * oh_lp_t25ia;
          oh_lp_a2 = (1.0 - oh_lp_t25al) * oh_lp_t25ia;
          oh_lp_z1 = 0.0;
          oh_lp_z2 = 0.0;
          oh_dcx = 0.0;
          oh_dcy = 0.0;
          oh_wa = 1.0 - dcoef(fs, 0.02);
          oh_wd = 0.0007 * std::sqrt(6.0 * oh_wa);
          oh_fdc = dcoef(fs, gmax(0.004, oh_dec * 0.01));
          oh_sdc = dcoef(fs, gmax(0.004, oh_dec * 0.58));
          oh_fe = 1.0;
          oh_se = 1.0;
          oh_ac = 1.0 - dcoef(fs, 0.0003);
          oh_ae = 0.0;
          oh_nd = gmax(0.05, oh_dec * 1.765623582766);
          oh_nf = gmax(1.0, std::floor(oh_nd * fs + 0.5));
          oh_gff = gmin(std::floor(0.5 * fs + 0.5), oh_nf);
          oh_igf = 0.0;
          if (oh_gff > 0.0) { oh_igf = 1.0 / oh_gff; }
          oh_fi = 0.0;
          oh_active = 1.0;
          oh_chk = 0.0;
          oh_chkg = 1.0;
        }
        if (oh_active > 0.0) {
          if (oh_fi >= oh_nf) { oh_active = 0.0; }
          else {
          oh_bg = 1.0 + 0.7 * oh_bae;
          oh_bae = oh_bae * oh_bac;
          oh_ton = 0.0;
          for (i = 0.0; i < 47.0; i = i + 1.0) {
            oh_wv = peek_(oh_wob, i);
            oh_wv = oh_wv + oh_wd * noise_() - oh_wa * oh_wv;
            poke_(oh_wob, oh_wv, i);
            oh_pv = peek_(oh_ph, i);
            oh_lg = 1.0;
            if (peek_(tbl, 862.0 + i) > 0.0) { oh_lg = oh_bg; }
            oh_ton = oh_ton + std::sin(oh_pv) * peek_(oh_amp, i) * oh_lg;
            oh_pv = oh_pv + peek_(oh_inc, i) * (1.0 + oh_wv);
            if (oh_pv >= 6.283185307179586) { oh_pv = oh_pv - 6.283185307179586; }
            poke_(oh_ph, oh_pv, i);
          }
          oh_nx = noise_();
          oh_wn = oh_nx - oh_nin + 0.995 * oh_nout;
          oh_nin = oh_nx;
          oh_nout = oh_wn;
          oh_n1 = oh_hp_b0 * oh_wn + oh_hp_z1;
          oh_hp_z1 = oh_hp_b1 * oh_wn - oh_hp_a1 * oh_n1 + oh_hp_z2;
          oh_hp_z2 = oh_hp_b2 * oh_wn - oh_hp_a2 * oh_n1;
          oh_nz = oh_lp_b0 * oh_n1 + oh_lp_z1;
          oh_lp_z1 = oh_lp_b1 * oh_n1 - oh_lp_a1 * oh_nz + oh_lp_z2;
          oh_lp_z2 = oh_lp_b2 * oh_n1 - oh_lp_a2 * oh_nz;
          oh_ae = oh_ae + oh_ac * (1.0 - oh_ae);
          oh_fe = oh_fe * oh_fdc;
          oh_se = oh_se * oh_sdc;
          oh_env = oh_ae * (0.11 * oh_fe + 0.89 * oh_se);
          if (oh_gff > 0.0) { oh_left = oh_nf - oh_fi; if (oh_left < oh_gff) { oh_env = oh_env * oh_left * oh_igf; } }
          oh_src = std::tanh((oh_ton * 0.11 + oh_nz * 0.909) * 0.6);
          oh_bd = oh_src * oh_env + oh_nz * 0.35 * oh_cke;
          oh_cke = oh_cke * oh_ckc;
          oh_o = oh_bd * 1.63;
          oh_y = oh_o - oh_dcx + 0.995 * oh_dcy;
          oh_dcx = oh_o;
          oh_dcy = oh_y;
          oh_out = oh_y;
          oh_fi = oh_fi + 1.0;
          if (oh_fi >= oh_nf) { oh_active = 0.0; }
          if (oh_chk > 0.0) {
            oh_chkg = oh_chkg * oh_chk;
            if (oh_chkg < 0.0001) { oh_active = 0.0; }
          }
          oh_out = oh_out * oh_chkg;
          }
        }
        // ---- cy ----
        cy_fire = 0.0;
        cy_vel = 0.0;
        if (tr_cy != cy_trlast) {
          cy_trlast = tr_cy;
          cy_vel = tr_cy - 128.0 * std::floor(tr_cy / 128.0);
          cy_fire = 1.0;
        }
        if (cy_sq > 0.0) { cy_vel = gmax(cy_vel, seq_vel); cy_fire = 1.0; }
        if (cy_fire > 0.0) {
          cy_vg = velgain(cy_vel, acc);
          cy_dec = gclamp(cy_decay * 0.01, 0.0, 1.0);
          cy_rt = gclamp(std::pow(2.0, cy_tune / 12.0), 0.25, 4.0);
          cy_tn = gclamp(cy_tone * 0.01, 0.0, 1.0);
          cy_gh = 0.55 + 0.9 * cy_tn;
          cy_gm = 1.35 - 0.9 * cy_tn;
          cy_p0 = (noise_() + 1.0) * 0.5;
          cy_p1 = (noise_() + 1.0) * 0.5;
          cy_p2 = (noise_() + 1.0) * 0.5;
          cy_p3 = (noise_() + 1.0) * 0.5;
          cy_p4 = (noise_() + 1.0) * 0.5;
          cy_p5 = (noise_() + 1.0) * 0.5;
          cy_ehc = t60coef(fs, (0.35 + 2.4 * cy_dec * cy_dec) / std::sqrt(cy_rt));
          cy_emc = t60coef(fs, (0.08 + 0.4 * cy_dec) / std::sqrt(cy_rt));
          cy_eh = 1.0;
          cy_em = 1.0;
          cy_rise = 0.0;
          cy_risec = 1.0 - opcoef(fs, 0.0004);
          cy_n = 0.0;
          cy_h1_t26c = gclamp(7100.0 * cy_rt, 10.0, fs * 0.45);
          cy_h1_t26q = gmax(0.05, 1.3);
          cy_h1_t26w = 6.283185307179586 * cy_h1_t26c / fs;
          cy_h1_t26cw = std::cos(cy_h1_t26w);
          cy_h1_t26sw = std::sin(cy_h1_t26w);
          cy_h1_t26al = cy_h1_t26sw / (2.0 * cy_h1_t26q);
          cy_h1_t26ia = 1.0 / (1.0 + cy_h1_t26al);
          cy_h1_b0 = 0.5 * cy_h1_t26sw * cy_h1_t26ia;
          cy_h1_b1 = 0.0;
          cy_h1_b2 = -0.5 * cy_h1_t26sw * cy_h1_t26ia;
          cy_h1_a1 = -2.0 * cy_h1_t26cw * cy_h1_t26ia;
          cy_h1_a2 = (1.0 - cy_h1_t26al) * cy_h1_t26ia;
          cy_h1_z1 = 0.0;
          cy_h1_z2 = 0.0;
          cy_h2_t27c = gclamp(8600.0 * cy_rt, 10.0, fs * 0.45);
          cy_h2_t27q = gmax(0.05, 1.1);
          cy_h2_t27w = 6.283185307179586 * cy_h2_t27c / fs;
          cy_h2_t27cw = std::cos(cy_h2_t27w);
          cy_h2_t27sw = std::sin(cy_h2_t27w);
          cy_h2_t27al = cy_h2_t27sw / (2.0 * cy_h2_t27q);
          cy_h2_t27ia = 1.0 / (1.0 + cy_h2_t27al);
          cy_h2_b0 = 0.5 * cy_h2_t27sw * cy_h2_t27ia;
          cy_h2_b1 = 0.0;
          cy_h2_b2 = -0.5 * cy_h2_t27sw * cy_h2_t27ia;
          cy_h2_a1 = -2.0 * cy_h2_t27cw * cy_h2_t27ia;
          cy_h2_a2 = (1.0 - cy_h2_t27al) * cy_h2_t27ia;
          cy_h2_z1 = 0.0;
          cy_h2_z2 = 0.0;
          cy_m1_t28c = gclamp(3400.0 * cy_rt, 10.0, fs * 0.45);
          cy_m1_t28q = gmax(0.05, 1.6);
          cy_m1_t28w = 6.283185307179586 * cy_m1_t28c / fs;
          cy_m1_t28cw = std::cos(cy_m1_t28w);
          cy_m1_t28sw = std::sin(cy_m1_t28w);
          cy_m1_t28al = cy_m1_t28sw / (2.0 * cy_m1_t28q);
          cy_m1_t28ia = 1.0 / (1.0 + cy_m1_t28al);
          cy_m1_b0 = 0.5 * cy_m1_t28sw * cy_m1_t28ia;
          cy_m1_b1 = 0.0;
          cy_m1_b2 = -0.5 * cy_m1_t28sw * cy_m1_t28ia;
          cy_m1_a1 = -2.0 * cy_m1_t28cw * cy_m1_t28ia;
          cy_m1_a2 = (1.0 - cy_m1_t28al) * cy_m1_t28ia;
          cy_m1_z1 = 0.0;
          cy_m1_z2 = 0.0;
          cy_m2_t29c = gclamp(3900.0 * cy_rt, 10.0, fs * 0.45);
          cy_m2_t29q = gmax(0.05, 1.4);
          cy_m2_t29w = 6.283185307179586 * cy_m2_t29c / fs;
          cy_m2_t29cw = std::cos(cy_m2_t29w);
          cy_m2_t29sw = std::sin(cy_m2_t29w);
          cy_m2_t29al = cy_m2_t29sw / (2.0 * cy_m2_t29q);
          cy_m2_t29ia = 1.0 / (1.0 + cy_m2_t29al);
          cy_m2_b0 = 0.5 * cy_m2_t29sw * cy_m2_t29ia;
          cy_m2_b1 = 0.0;
          cy_m2_b2 = -0.5 * cy_m2_t29sw * cy_m2_t29ia;
          cy_m2_a1 = -2.0 * cy_m2_t29cw * cy_m2_t29ia;
          cy_m2_a2 = (1.0 - cy_m2_t29al) * cy_m2_t29ia;
          cy_m2_z1 = 0.0;
          cy_m2_z2 = 0.0;
          cy_hp_t30c = gclamp(2600.0 * cy_rt, 10.0, fs * 0.45);
          cy_hp_t30q = gmax(0.05, 0.7071);
          cy_hp_t30w = 6.283185307179586 * cy_hp_t30c / fs;
          cy_hp_t30cw = std::cos(cy_hp_t30w);
          cy_hp_t30sw = std::sin(cy_hp_t30w);
          cy_hp_t30al = cy_hp_t30sw / (2.0 * cy_hp_t30q);
          cy_hp_t30ia = 1.0 / (1.0 + cy_hp_t30al);
          cy_hp_b0 = (1.0 + cy_hp_t30cw) * 0.5 * cy_hp_t30ia;
          cy_hp_b1 = -(1.0 + cy_hp_t30cw) * cy_hp_t30ia;
          cy_hp_b2 = cy_hp_b0;
          cy_hp_a1 = -2.0 * cy_hp_t30cw * cy_hp_t30ia;
          cy_hp_a2 = (1.0 - cy_hp_t30al) * cy_hp_t30ia;
          cy_hp_z1 = 0.0;
          cy_hp_z2 = 0.0;
          cy_active = 1.0;
        }
        if (cy_active > 0.0) {
          cy_sq = 0.0;
          cy_dt0 = gmin(245.0 * cy_rt / fs, 0.45);
          cy_p0 = cy_p0 + cy_dt0;
          if (cy_p0 >= 1.0) { cy_p0 = cy_p0 - 1.0; }
          cy_hf0 = cy_p0 + 0.5;
          if (cy_hf0 >= 1.0) { cy_hf0 = cy_hf0 - 1.0; }
          cy_nv0 = 1.0;
          if (cy_p0 >= 0.5) { cy_nv0 = -1.0; }
          cy_sq = cy_sq + cy_nv0 + blep(cy_p0, cy_dt0) - blep(cy_hf0, cy_dt0);
          cy_dt1 = gmin(306.0 * cy_rt / fs, 0.45);
          cy_p1 = cy_p1 + cy_dt1;
          if (cy_p1 >= 1.0) { cy_p1 = cy_p1 - 1.0; }
          cy_hf1 = cy_p1 + 0.5;
          if (cy_hf1 >= 1.0) { cy_hf1 = cy_hf1 - 1.0; }
          cy_nv1 = 1.0;
          if (cy_p1 >= 0.5) { cy_nv1 = -1.0; }
          cy_sq = cy_sq + cy_nv1 + blep(cy_p1, cy_dt1) - blep(cy_hf1, cy_dt1);
          cy_dt2 = gmin(365.0 * cy_rt / fs, 0.45);
          cy_p2 = cy_p2 + cy_dt2;
          if (cy_p2 >= 1.0) { cy_p2 = cy_p2 - 1.0; }
          cy_hf2 = cy_p2 + 0.5;
          if (cy_hf2 >= 1.0) { cy_hf2 = cy_hf2 - 1.0; }
          cy_nv2 = 1.0;
          if (cy_p2 >= 0.5) { cy_nv2 = -1.0; }
          cy_sq = cy_sq + cy_nv2 + blep(cy_p2, cy_dt2) - blep(cy_hf2, cy_dt2);
          cy_dt3 = gmin(415.0 * cy_rt / fs, 0.45);
          cy_p3 = cy_p3 + cy_dt3;
          if (cy_p3 >= 1.0) { cy_p3 = cy_p3 - 1.0; }
          cy_hf3 = cy_p3 + 0.5;
          if (cy_hf3 >= 1.0) { cy_hf3 = cy_hf3 - 1.0; }
          cy_nv3 = 1.0;
          if (cy_p3 >= 0.5) { cy_nv3 = -1.0; }
          cy_sq = cy_sq + cy_nv3 + blep(cy_p3, cy_dt3) - blep(cy_hf3, cy_dt3);
          cy_dt4 = gmin(437.0 * cy_rt / fs, 0.45);
          cy_p4 = cy_p4 + cy_dt4;
          if (cy_p4 >= 1.0) { cy_p4 = cy_p4 - 1.0; }
          cy_hf4 = cy_p4 + 0.5;
          if (cy_hf4 >= 1.0) { cy_hf4 = cy_hf4 - 1.0; }
          cy_nv4 = 1.0;
          if (cy_p4 >= 0.5) { cy_nv4 = -1.0; }
          cy_sq = cy_sq + cy_nv4 + blep(cy_p4, cy_dt4) - blep(cy_hf4, cy_dt4);
          cy_dt5 = gmin(619.0 * cy_rt / fs, 0.45);
          cy_p5 = cy_p5 + cy_dt5;
          if (cy_p5 >= 1.0) { cy_p5 = cy_p5 - 1.0; }
          cy_hf5 = cy_p5 + 0.5;
          if (cy_hf5 >= 1.0) { cy_hf5 = cy_hf5 - 1.0; }
          cy_nv5 = 1.0;
          if (cy_p5 >= 0.5) { cy_nv5 = -1.0; }
          cy_sq = cy_sq + cy_nv5 + blep(cy_p5, cy_dt5) - blep(cy_hf5, cy_dt5);
          cy_sq = cy_sq * 0.16667;
          cy_a1 = cy_h1_b0 * cy_sq + cy_h1_z1;
          cy_h1_z1 = cy_h1_b1 * cy_sq - cy_h1_a1 * cy_a1 + cy_h1_z2;
          cy_h1_z2 = cy_h1_b2 * cy_sq - cy_h1_a2 * cy_a1;
          cy_a2 = cy_h2_b0 * cy_a1 + cy_h2_z1;
          cy_h2_z1 = cy_h2_b1 * cy_a1 - cy_h2_a1 * cy_a2 + cy_h2_z2;
          cy_h2_z2 = cy_h2_b2 * cy_a1 - cy_h2_a2 * cy_a2;
          cy_b1 = cy_m1_b0 * cy_sq + cy_m1_z1;
          cy_m1_z1 = cy_m1_b1 * cy_sq - cy_m1_a1 * cy_b1 + cy_m1_z2;
          cy_m1_z2 = cy_m1_b2 * cy_sq - cy_m1_a2 * cy_b1;
          cy_b2 = cy_m2_b0 * cy_b1 + cy_m2_z1;
          cy_m2_z1 = cy_m2_b1 * cy_b1 - cy_m2_a1 * cy_b2 + cy_m2_z2;
          cy_m2_z2 = cy_m2_b2 * cy_b1 - cy_m2_a2 * cy_b2;
          cy_rise = cy_rise + (1.0 - cy_rise) * cy_risec;
          cy_tm = cy_n / fs;
          cy_tr = 1.0 + 1.5 * std::exp(-cy_tm / 0.012);
          cy_mix = (cy_a2 * cy_eh * cy_gh * cy_tr + cy_b2 * cy_em * cy_gm) * cy_rise;
          cy_hpo = cy_hp_b0 * cy_mix + cy_hp_z1;
          cy_hp_z1 = cy_hp_b1 * cy_mix - cy_hp_a1 * cy_hpo + cy_hp_z2;
          cy_hp_z2 = cy_hp_b2 * cy_mix - cy_hp_a2 * cy_hpo;
          cy_out = std::tanh(cy_hpo * 6.0) * 1.0;
          cy_eh = cy_eh * cy_ehc;
          cy_em = cy_em * cy_emc;
          cy_n = cy_n + 1.0;
          if (cy_eh < 0.0001 && cy_em < 0.0001) { cy_active = 0.0; }
        }
        anysolo = solo_bd + solo_sd + solo_lt + solo_ht + solo_cp + solo_ch + solo_oh + solo_cy;
        bd_mt = 1.0;
        if (mute_bd > 0.0 || (anysolo > 0.0 && solo_bd <= 0.0)) { bd_mt = 0.0; }
        bd_mg = bd_mg + (bd_mt - bd_mg) * 0.004;
        sd_mt = 1.0;
        if (mute_sd > 0.0 || (anysolo > 0.0 && solo_sd <= 0.0)) { sd_mt = 0.0; }
        sd_mg = sd_mg + (sd_mt - sd_mg) * 0.004;
        lt_mt = 1.0;
        if (mute_lt > 0.0 || (anysolo > 0.0 && solo_lt <= 0.0)) { lt_mt = 0.0; }
        lt_mg = lt_mg + (lt_mt - lt_mg) * 0.004;
        ht_mt = 1.0;
        if (mute_ht > 0.0 || (anysolo > 0.0 && solo_ht <= 0.0)) { ht_mt = 0.0; }
        ht_mg = ht_mg + (ht_mt - ht_mg) * 0.004;
        cp_mt = 1.0;
        if (mute_cp > 0.0 || (anysolo > 0.0 && solo_cp <= 0.0)) { cp_mt = 0.0; }
        cp_mg = cp_mg + (cp_mt - cp_mg) * 0.004;
        ch_mt = 1.0;
        if (mute_ch > 0.0 || (anysolo > 0.0 && solo_ch <= 0.0)) { ch_mt = 0.0; }
        ch_mg = ch_mg + (ch_mt - ch_mg) * 0.004;
        oh_mt = 1.0;
        if (mute_oh > 0.0 || (anysolo > 0.0 && solo_oh <= 0.0)) { oh_mt = 0.0; }
        oh_mg = oh_mg + (oh_mt - oh_mg) * 0.004;
        cy_mt = 1.0;
        if (mute_cy > 0.0 || (anysolo > 0.0 && solo_cy <= 0.0)) { cy_mt = 0.0; }
        cy_mg = cy_mg + (cy_mt - cy_mg) * 0.004;
        // each drum after its level, accent and mute/solo, before the master volume
        vout[0] = bd_out * bd_vg * lvlgain(bd_lvl) * bd_mg;
        vout[1] = sd_out * sd_vg * lvlgain(sd_lvl) * sd_mg;
        vout[2] = lt_out * lt_vg * lvlgain(lt_lvl) * lt_mg;
        vout[3] = ht_out * ht_vg * lvlgain(ht_lvl) * ht_mg;
        vout[4] = cp_out * cp_vg * lvlgain(cp_lvl) * cp_mg;
        vout[5] = ch_out * ch_vg * lvlgain(ch_lvl) * ch_mg;
        vout[6] = oh_out * oh_vg * lvlgain(oh_lvl) * oh_mg;
        vout[7] = cy_out * cy_vg * lvlgain(cy_lvl) * cy_mg;
        total = (vout[0] + vout[1] + vout[2] + vout[3] + vout[4] + vout[5] + vout[6] + vout[7]) * lvlgain(master);
        h_ticklast = ticklast;
        h_bd_mg = bd_mg;
        h_sd_mg = sd_mg;
        h_lt_mg = lt_mg;
        h_ht_mg = ht_mg;
        h_cp_mg = cp_mg;
        h_ch_mg = ch_mg;
        h_oh_mg = oh_mg;
        h_cy_mg = cy_mg;
        h_bd_active = bd_active;
        h_bd_trlast = bd_trlast;
        h_bd_vg = bd_vg;
        h_bd_sil = bd_sil;
        h_bd_ph = bd_ph;
        h_bd_hz = bd_hz;
        h_bd_tg = bd_tg;
        h_bd_amp = bd_amp;
        h_bd_ampc = bd_ampc;
        h_bd_pc = bd_pc;
        h_bd_ce = bd_ce;
        h_bd_cc = bd_cc;
        h_bd_ie = bd_ie;
        h_bd_ic = bd_ic;
        h_bd_clz = bd_clz;
        h_bd_cla = bd_cla;
        h_bd_blz = bd_blz;
        h_bd_bla = bd_bla;
        h_bd_ilz = bd_ilz;
        h_bd_ila = bd_ila;
        h_bd_dcx = bd_dcx;
        h_bd_dcy = bd_dcy;
        h_bd_last = bd_last;
        h_bd_nin = bd_nin;
        h_bd_nout = bd_nout;
        h_bd_amt = bd_amt;
        h_bd_hp_b0 = bd_hp_b0;
        h_bd_hp_b1 = bd_hp_b1;
        h_bd_hp_b2 = bd_hp_b2;
        h_bd_hp_a1 = bd_hp_a1;
        h_bd_hp_a2 = bd_hp_a2;
        h_bd_hp_z1 = bd_hp_z1;
        h_bd_hp_z2 = bd_hp_z2;
        h_sd_active = sd_active;
        h_sd_trlast = sd_trlast;
        h_sd_vg = sd_vg;
        h_sd_fi = sd_fi;
        h_sd_nf = sd_nf;
        h_sd_bph = sd_bph;
        h_sd_rph = sd_rph;
        h_sd_dec = sd_dec;
        h_sd_pr = sd_pr;
        h_sd_snap = sd_snap;
        h_sd_col = sd_col;
        h_sd_sel = sd_sel;
        h_sd_ghold = sd_ghold;
        h_sd_gfade = sd_gfade;
        h_sd_ngain = sd_ngain;
        h_sd_lpz = sd_lpz;
        h_sd_lpa = sd_lpa;
        h_sd_b1_b0 = sd_b1_b0;
        h_sd_b1_b1 = sd_b1_b1;
        h_sd_b1_b2 = sd_b1_b2;
        h_sd_b1_a1 = sd_b1_a1;
        h_sd_b1_a2 = sd_b1_a2;
        h_sd_b1_z1 = sd_b1_z1;
        h_sd_b1_z2 = sd_b1_z2;
        h_sd_b2_b0 = sd_b2_b0;
        h_sd_b2_b1 = sd_b2_b1;
        h_sd_b2_b2 = sd_b2_b2;
        h_sd_b2_a1 = sd_b2_a1;
        h_sd_b2_a2 = sd_b2_a2;
        h_sd_b2_z1 = sd_b2_z1;
        h_sd_b2_z2 = sd_b2_z2;
        h_lt_active = lt_active;
        h_lt_trlast = lt_trlast;
        h_lt_vg = lt_vg;
        h_lt_mhz = lt_mhz;
        h_lt_glide = lt_glide;
        h_lt_gpole = lt_gpole;
        h_lt_lhz = lt_lhz;
        h_lt_shz = lt_shz;
        h_lt_mph = lt_mph;
        h_lt_lph = lt_lph;
        h_lt_sph = lt_sph;
        h_lt_me = lt_me;
        h_lt_le = lt_le;
        h_lt_se = lt_se;
        h_lt_sne = lt_sne;
        h_lt_bue = lt_bue;
        h_lt_tne = lt_tne;
        h_lt_mpole = lt_mpole;
        h_lt_lpole = lt_lpole;
        h_lt_spole = lt_spole;
        h_lt_snp = lt_snp;
        h_lt_bup = lt_bup;
        h_lt_tnp = lt_tnp;
        h_lt_br = lt_br;
        h_lt_brc = lt_brc;
        h_lt_sr = lt_sr;
        h_lt_src = lt_src;
        h_lt_snr = lt_snr;
        h_lt_snrc = lt_snrc;
        h_lt_nr = lt_nr;
        h_lt_nrc = lt_nrc;
        h_lt_lnh_b0 = lt_lnh_b0;
        h_lt_lnh_b1 = lt_lnh_b1;
        h_lt_lnh_b2 = lt_lnh_b2;
        h_lt_lnh_a1 = lt_lnh_a1;
        h_lt_lnh_a2 = lt_lnh_a2;
        h_lt_lnh_z1 = lt_lnh_z1;
        h_lt_lnh_z2 = lt_lnh_z2;
        h_lt_lnl_b0 = lt_lnl_b0;
        h_lt_lnl_b1 = lt_lnl_b1;
        h_lt_lnl_b2 = lt_lnl_b2;
        h_lt_lnl_a1 = lt_lnl_a1;
        h_lt_lnl_a2 = lt_lnl_a2;
        h_lt_lnl_z1 = lt_lnl_z1;
        h_lt_lnl_z2 = lt_lnl_z2;
        h_lt_hnh_b0 = lt_hnh_b0;
        h_lt_hnh_b1 = lt_hnh_b1;
        h_lt_hnh_b2 = lt_hnh_b2;
        h_lt_hnh_a1 = lt_hnh_a1;
        h_lt_hnh_a2 = lt_hnh_a2;
        h_lt_hnh_z1 = lt_hnh_z1;
        h_lt_hnh_z2 = lt_hnh_z2;
        h_lt_hnl_b0 = lt_hnl_b0;
        h_lt_hnl_b1 = lt_hnl_b1;
        h_lt_hnl_b2 = lt_hnl_b2;
        h_lt_hnl_a1 = lt_hnl_a1;
        h_lt_hnl_a2 = lt_hnl_a2;
        h_lt_hnl_z1 = lt_hnl_z1;
        h_lt_hnl_z2 = lt_hnl_z2;
        h_lt_tnh_b0 = lt_tnh_b0;
        h_lt_tnh_b1 = lt_tnh_b1;
        h_lt_tnh_b2 = lt_tnh_b2;
        h_lt_tnh_a1 = lt_tnh_a1;
        h_lt_tnh_a2 = lt_tnh_a2;
        h_lt_tnh_z1 = lt_tnh_z1;
        h_lt_tnh_z2 = lt_tnh_z2;
        h_lt_tnl_b0 = lt_tnl_b0;
        h_lt_tnl_b1 = lt_tnl_b1;
        h_lt_tnl_b2 = lt_tnl_b2;
        h_lt_tnl_a1 = lt_tnl_a1;
        h_lt_tnl_a2 = lt_tnl_a2;
        h_lt_tnl_z1 = lt_tnl_z1;
        h_lt_tnl_z2 = lt_tnl_z2;
        h_lt_fsh_b0 = lt_fsh_b0;
        h_lt_fsh_b1 = lt_fsh_b1;
        h_lt_fsh_b2 = lt_fsh_b2;
        h_lt_fsh_a1 = lt_fsh_a1;
        h_lt_fsh_a2 = lt_fsh_a2;
        h_lt_fsh_z1 = lt_fsh_z1;
        h_lt_fsh_z2 = lt_fsh_z2;
        h_lt_fsl_b0 = lt_fsl_b0;
        h_lt_fsl_b1 = lt_fsl_b1;
        h_lt_fsl_b2 = lt_fsl_b2;
        h_lt_fsl_a1 = lt_fsl_a1;
        h_lt_fsl_a2 = lt_fsl_a2;
        h_lt_fsl_z1 = lt_fsl_z1;
        h_lt_fsl_z2 = lt_fsl_z2;
        h_ht_active = ht_active;
        h_ht_trlast = ht_trlast;
        h_ht_vg = ht_vg;
        h_ht_mhz = ht_mhz;
        h_ht_glide = ht_glide;
        h_ht_gpole = ht_gpole;
        h_ht_lhz = ht_lhz;
        h_ht_shz = ht_shz;
        h_ht_mph = ht_mph;
        h_ht_lph = ht_lph;
        h_ht_sph = ht_sph;
        h_ht_me = ht_me;
        h_ht_le = ht_le;
        h_ht_se = ht_se;
        h_ht_sne = ht_sne;
        h_ht_bue = ht_bue;
        h_ht_tne = ht_tne;
        h_ht_mpole = ht_mpole;
        h_ht_lpole = ht_lpole;
        h_ht_spole = ht_spole;
        h_ht_snp = ht_snp;
        h_ht_bup = ht_bup;
        h_ht_tnp = ht_tnp;
        h_ht_br = ht_br;
        h_ht_brc = ht_brc;
        h_ht_sr = ht_sr;
        h_ht_src = ht_src;
        h_ht_snr = ht_snr;
        h_ht_snrc = ht_snrc;
        h_ht_nr = ht_nr;
        h_ht_nrc = ht_nrc;
        h_ht_uph0 = ht_uph0;
        h_ht_uhz0 = ht_uhz0;
        h_ht_ue0 = ht_ue0;
        h_ht_up0 = ht_up0;
        h_ht_uph1 = ht_uph1;
        h_ht_uhz1 = ht_uhz1;
        h_ht_ue1 = ht_ue1;
        h_ht_up1 = ht_up1;
        h_ht_uph2 = ht_uph2;
        h_ht_uhz2 = ht_uhz2;
        h_ht_ue2 = ht_ue2;
        h_ht_up2 = ht_up2;
        h_ht_uph3 = ht_uph3;
        h_ht_uhz3 = ht_uhz3;
        h_ht_ue3 = ht_ue3;
        h_ht_up3 = ht_up3;
        h_ht_lnh_b0 = ht_lnh_b0;
        h_ht_lnh_b1 = ht_lnh_b1;
        h_ht_lnh_b2 = ht_lnh_b2;
        h_ht_lnh_a1 = ht_lnh_a1;
        h_ht_lnh_a2 = ht_lnh_a2;
        h_ht_lnh_z1 = ht_lnh_z1;
        h_ht_lnh_z2 = ht_lnh_z2;
        h_ht_lnl_b0 = ht_lnl_b0;
        h_ht_lnl_b1 = ht_lnl_b1;
        h_ht_lnl_b2 = ht_lnl_b2;
        h_ht_lnl_a1 = ht_lnl_a1;
        h_ht_lnl_a2 = ht_lnl_a2;
        h_ht_lnl_z1 = ht_lnl_z1;
        h_ht_lnl_z2 = ht_lnl_z2;
        h_ht_hnh_b0 = ht_hnh_b0;
        h_ht_hnh_b1 = ht_hnh_b1;
        h_ht_hnh_b2 = ht_hnh_b2;
        h_ht_hnh_a1 = ht_hnh_a1;
        h_ht_hnh_a2 = ht_hnh_a2;
        h_ht_hnh_z1 = ht_hnh_z1;
        h_ht_hnh_z2 = ht_hnh_z2;
        h_ht_hnl_b0 = ht_hnl_b0;
        h_ht_hnl_b1 = ht_hnl_b1;
        h_ht_hnl_b2 = ht_hnl_b2;
        h_ht_hnl_a1 = ht_hnl_a1;
        h_ht_hnl_a2 = ht_hnl_a2;
        h_ht_hnl_z1 = ht_hnl_z1;
        h_ht_hnl_z2 = ht_hnl_z2;
        h_ht_tnh_b0 = ht_tnh_b0;
        h_ht_tnh_b1 = ht_tnh_b1;
        h_ht_tnh_b2 = ht_tnh_b2;
        h_ht_tnh_a1 = ht_tnh_a1;
        h_ht_tnh_a2 = ht_tnh_a2;
        h_ht_tnh_z1 = ht_tnh_z1;
        h_ht_tnh_z2 = ht_tnh_z2;
        h_ht_tnl_b0 = ht_tnl_b0;
        h_ht_tnl_b1 = ht_tnl_b1;
        h_ht_tnl_b2 = ht_tnl_b2;
        h_ht_tnl_a1 = ht_tnl_a1;
        h_ht_tnl_a2 = ht_tnl_a2;
        h_ht_tnl_z1 = ht_tnl_z1;
        h_ht_tnl_z2 = ht_tnl_z2;
        h_cp_active = cp_active;
        h_cp_trlast = cp_trlast;
        h_cp_vg = cp_vg;
        h_cp_si = cp_si;
        h_cp_maxs = cp_maxs;
        h_cp_dec = cp_dec;
        h_cp_air = cp_air;
        h_cp_nlg = cp_nlg;
        h_cp_ahc = cp_ahc;
        h_cp_ahi = cp_ahi;
        h_cp_aho = cp_aho;
        h_cp_tapn = cp_tapn;
        h_cp_pos = cp_pos;
        h_cp_bl = cp_bl;
        h_cp_la1_b0 = cp_la1_b0;
        h_cp_la1_b1 = cp_la1_b1;
        h_cp_la1_b2 = cp_la1_b2;
        h_cp_la1_a1 = cp_la1_a1;
        h_cp_la1_a2 = cp_la1_a2;
        h_cp_la1_z1 = cp_la1_z1;
        h_cp_la1_z2 = cp_la1_z2;
        h_cp_la2_b0 = cp_la2_b0;
        h_cp_la2_b1 = cp_la2_b1;
        h_cp_la2_b2 = cp_la2_b2;
        h_cp_la2_a1 = cp_la2_a1;
        h_cp_la2_a2 = cp_la2_a2;
        h_cp_la2_z1 = cp_la2_z1;
        h_cp_la2_z2 = cp_la2_z2;
        h_cp_lb1_b0 = cp_lb1_b0;
        h_cp_lb1_b1 = cp_lb1_b1;
        h_cp_lb1_b2 = cp_lb1_b2;
        h_cp_lb1_a1 = cp_lb1_a1;
        h_cp_lb1_a2 = cp_lb1_a2;
        h_cp_lb1_z1 = cp_lb1_z1;
        h_cp_lb1_z2 = cp_lb1_z2;
        h_cp_lb2_b0 = cp_lb2_b0;
        h_cp_lb2_b1 = cp_lb2_b1;
        h_cp_lb2_b2 = cp_lb2_b2;
        h_cp_lb2_a1 = cp_lb2_a1;
        h_cp_lb2_a2 = cp_lb2_a2;
        h_cp_lb2_z1 = cp_lb2_z1;
        h_cp_lb2_z2 = cp_lb2_z2;
        h_ch_active = ch_active;
        h_ch_trlast = ch_trlast;
        h_ch_vg = ch_vg;
        h_ch_fi = ch_fi;
        h_ch_nf = ch_nf;
        h_ch_gff = ch_gff;
        h_ch_igf = ch_igf;
        h_ch_bae = ch_bae;
        h_ch_bac = ch_bac;
        h_ch_cke = ch_cke;
        h_ch_ckc = ch_ckc;
        h_ch_dcx = ch_dcx;
        h_ch_dcy = ch_dcy;
        h_ch_wa = ch_wa;
        h_ch_wd = ch_wd;
        h_ch_fe = ch_fe;
        h_ch_se = ch_se;
        h_ch_fdc = ch_fdc;
        h_ch_sdc = ch_sdc;
        h_ch_ae = ch_ae;
        h_ch_ac = ch_ac;
        h_ch_nin = ch_nin;
        h_ch_nout = ch_nout;
        h_ch_hp_b0 = ch_hp_b0;
        h_ch_hp_b1 = ch_hp_b1;
        h_ch_hp_b2 = ch_hp_b2;
        h_ch_hp_a1 = ch_hp_a1;
        h_ch_hp_a2 = ch_hp_a2;
        h_ch_hp_z1 = ch_hp_z1;
        h_ch_hp_z2 = ch_hp_z2;
        h_ch_lp_b0 = ch_lp_b0;
        h_ch_lp_b1 = ch_lp_b1;
        h_ch_lp_b2 = ch_lp_b2;
        h_ch_lp_a1 = ch_lp_a1;
        h_ch_lp_a2 = ch_lp_a2;
        h_ch_lp_z1 = ch_lp_z1;
        h_ch_lp_z2 = ch_lp_z2;
        h_oh_active = oh_active;
        h_oh_trlast = oh_trlast;
        h_oh_vg = oh_vg;
        h_oh_fi = oh_fi;
        h_oh_nf = oh_nf;
        h_oh_gff = oh_gff;
        h_oh_igf = oh_igf;
        h_oh_bae = oh_bae;
        h_oh_bac = oh_bac;
        h_oh_cke = oh_cke;
        h_oh_ckc = oh_ckc;
        h_oh_dcx = oh_dcx;
        h_oh_dcy = oh_dcy;
        h_oh_wa = oh_wa;
        h_oh_wd = oh_wd;
        h_oh_fe = oh_fe;
        h_oh_se = oh_se;
        h_oh_fdc = oh_fdc;
        h_oh_sdc = oh_sdc;
        h_oh_ae = oh_ae;
        h_oh_ac = oh_ac;
        h_oh_nin = oh_nin;
        h_oh_nout = oh_nout;
        h_oh_hp_b0 = oh_hp_b0;
        h_oh_hp_b1 = oh_hp_b1;
        h_oh_hp_b2 = oh_hp_b2;
        h_oh_hp_a1 = oh_hp_a1;
        h_oh_hp_a2 = oh_hp_a2;
        h_oh_hp_z1 = oh_hp_z1;
        h_oh_hp_z2 = oh_hp_z2;
        h_oh_lp_b0 = oh_lp_b0;
        h_oh_lp_b1 = oh_lp_b1;
        h_oh_lp_b2 = oh_lp_b2;
        h_oh_lp_a1 = oh_lp_a1;
        h_oh_lp_a2 = oh_lp_a2;
        h_oh_lp_z1 = oh_lp_z1;
        h_oh_lp_z2 = oh_lp_z2;
        h_oh_chk = oh_chk;
        h_oh_chkg = oh_chkg;
        h_cy_active = cy_active;
        h_cy_trlast = cy_trlast;
        h_cy_vg = cy_vg;
        h_cy_p0 = cy_p0;
        h_cy_p1 = cy_p1;
        h_cy_p2 = cy_p2;
        h_cy_p3 = cy_p3;
        h_cy_p4 = cy_p4;
        h_cy_p5 = cy_p5;
        h_cy_rt = cy_rt;
        h_cy_eh = cy_eh;
        h_cy_em = cy_em;
        h_cy_ehc = cy_ehc;
        h_cy_emc = cy_emc;
        h_cy_rise = cy_rise;
        h_cy_risec = cy_risec;
        h_cy_n = cy_n;
        h_cy_gh = cy_gh;
        h_cy_gm = cy_gm;
        h_cy_dec = cy_dec;
        h_cy_h1_b0 = cy_h1_b0;
        h_cy_h1_b1 = cy_h1_b1;
        h_cy_h1_b2 = cy_h1_b2;
        h_cy_h1_a1 = cy_h1_a1;
        h_cy_h1_a2 = cy_h1_a2;
        h_cy_h1_z1 = cy_h1_z1;
        h_cy_h1_z2 = cy_h1_z2;
        h_cy_h2_b0 = cy_h2_b0;
        h_cy_h2_b1 = cy_h2_b1;
        h_cy_h2_b2 = cy_h2_b2;
        h_cy_h2_a1 = cy_h2_a1;
        h_cy_h2_a2 = cy_h2_a2;
        h_cy_h2_z1 = cy_h2_z1;
        h_cy_h2_z2 = cy_h2_z2;
        h_cy_m1_b0 = cy_m1_b0;
        h_cy_m1_b1 = cy_m1_b1;
        h_cy_m1_b2 = cy_m1_b2;
        h_cy_m1_a1 = cy_m1_a1;
        h_cy_m1_a2 = cy_m1_a2;
        h_cy_m1_z1 = cy_m1_z1;
        h_cy_m1_z2 = cy_m1_z2;
        h_cy_m2_b0 = cy_m2_b0;
        h_cy_m2_b1 = cy_m2_b1;
        h_cy_m2_b2 = cy_m2_b2;
        h_cy_m2_a1 = cy_m2_a1;
        h_cy_m2_a2 = cy_m2_a2;
        h_cy_m2_z1 = cy_m2_z1;
        h_cy_m2_z2 = cy_m2_z2;
        h_cy_hp_b0 = cy_hp_b0;
        h_cy_hp_b1 = cy_hp_b1;
        h_cy_hp_b2 = cy_hp_b2;
        h_cy_hp_a1 = cy_hp_a1;
        h_cy_hp_a2 = cy_hp_a2;
        h_cy_hp_z1 = cy_hp_z1;
        h_cy_hp_z2 = cy_hp_z2;
        return total;
    }
};

} // namespace tross606

#if defined (__clang__)
 #pragma clang diagnostic pop
#elif defined (__GNUC__)
 #pragma GCC diagnostic pop
#elif defined (_MSC_VER)
 #pragma warning (pop)
#endif
