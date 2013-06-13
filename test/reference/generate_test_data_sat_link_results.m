#!/usr/bin/octave -qf
# -*- Mode: Octave -*-

global g_step = 0.1;

function bler = get_bler (sinr_list, bler_list, sinr)
  if (sinr < sinr_list(1))
    bler = 1.0;
  elseif (sinr > sinr_list(end))
    bler = 0.0;
  else
    bler = interp1 (sinr_list, bler_list, sinr, 'linear');
  endif  
endfunction

function print_test_case (link_results_path, dvb_type, modcod, burst_length)
  global g_step;
  
  printf ("Ptr<SatLinkResults%s> linkResults%s ", dvb_type, dvb_type);
  printf ("= CreateObject<SatLinkResults%s> ();\n", dvb_type);
  printf ("linkResults%s->Initialize ();\n", dvb_type);
  
  link_results = load (strcat ("../../data/linkresults/", link_results_path));
  sinr_list = link_results(:,1);
  bler_list = link_results(:,2);
  sinr_min = floor (sinr_list(1)/g_step) * g_step;
  sinr_max = ceil (sinr_list(end)/g_step) * g_step;
  
  for sinr = [sinr_min:g_step:sinr_max]
    printf ("AddTestCase (new SatLinkResults%sTestCase ", dvb_type);
    printf ("(linkResults%s, ", dvb_type);
    printf ("SatLinkResults::SAT_MODCOD_%s, %d, ", modcod, burst_length);
    printf ("%f, %e)", sinr, get_bler (sinr_list, bler_list, sinr));
    printf (");\n");    
  endfor
  
endfunction

# print_test_case ("rcs2_waveformat10.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat11.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat12.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat13.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat14.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat15.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat16.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat17.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat18.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat19.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat20.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat21.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat22.txt", "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat2.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat3.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat4.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat5.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat6.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat7.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat8.txt",  "DvbRcs2", "", 0);
# print_test_case ("rcs2_waveformat9.txt",  "DvbRcs2", "", 0);

print_test_case ("s2_16apsk_2_to_3.txt", "DvbS2", "16APSK_2_TO_3", 626);

# print_test_case ("s2_16apsk_3_to_4.txt",  "DvbS2", "", 0);
# print_test_case ("s2_16apsk_4_to_5.txt",  "DvbS2", "", 0);
# print_test_case ("s2_16apsk_5_to_6.txt",  "DvbS2", "", 0);
# print_test_case ("s2_16apsk_8_to_9.txt",  "DvbS2", "", 0);
# print_test_case ("s2_16apsk_9_to_10.txt", "DvbS2", "", 0);
# print_test_case ("s2_32apsk_3_to_4.txt",  "DvbS2", "", 0);
# print_test_case ("s2_32apsk_4_to_5.txt",  "DvbS2", "", 0);
# print_test_case ("s2_32apsk_5_to_6.txt",  "DvbS2", "", 0);
# print_test_case ("s2_32apsk_8_to_9.txt",  "DvbS2", "", 0);
# print_test_case ("s2_8psk_2_to_3.txt",    "DvbS2", "", 0);
# print_test_case ("s2_8psk_3_to_4.txt",    "DvbS2", "", 0);
# print_test_case ("s2_8psk_3_to_5.txt",    "DvbS2", "", 0);
# print_test_case ("s2_8psk_5_to_6.txt",    "DvbS2", "", 0);
# print_test_case ("s2_8psk_8_to_9.txt",    "DvbS2", "", 0);
# print_test_case ("s2_8psk_9_to_10.txt",   "DvbS2", "", 0);
# print_test_case ("s2_qpsk_1_to_2.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_2_to_3.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_3_to_4.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_3_to_5.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_4_to_5.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_5_to_6.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_8_to_9.txt",    "DvbS2", "", 0);
# print_test_case ("s2_qpsk_9_to_10.txt",   "DvbS2", "", 0);

