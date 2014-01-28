#!/usr/bin/octave -qf
# -*- Mode: Octave -*-
#
# Copyright (c) 2013 Magister Solutions
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation;
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Budiarto Herman <budiarto.herman@magister.fi>
#

### WARNING: this file has not been compatible anymore since revision 9471 ###

global g_step_db = 0.1;

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
  global g_step_db;
  
  link_results = load (strcat ("../../data/linkresults/", link_results_path));
  sinr_list_db = link_results(:, 1);
  bler_list = link_results(:, 2);
  sinr_min = floor (sinr_list_db(1) / g_step_db) * g_step_db;
  sinr_max = ceil (sinr_list_db(end) / g_step_db) * g_step_db;
  
  for sinr_db = [sinr_min : g_step_db : sinr_max]
    printf ("AddTestCase (new SatLinkResults%sTestCase ", dvb_type);
    printf ("(linkResults%s, ", dvb_type);
    printf ("SatLinkResults::SAT_MODCOD_%s, %d, ", modcod, burst_length);
    printf ("%f, %e)", sinr_db, get_bler (sinr_list_db, bler_list, sinr_db));
    printf (", TestCase::QUICK);\n");    
  endfor

  printf ("\n");
endfunction

### DVB-RCS2 test cases ###

printf ("Ptr<SatLinkResultsDvbRcs2> linkResultsDvbRcs2 ");
printf ("= CreateObject<SatLinkResultsDvbRcs2> ();\n");
printf ("linkResultsDvbRcs2->Initialize ();\n");
printf ("\n");

print_test_case ("rcs2_waveformat2.txt", "DvbRcs2", "QPSK_1_TO_3", 262);
print_test_case ("rcs2_waveformat3.txt", "DvbRcs2", "QPSK_1_TO_3", 536);
print_test_case ("rcs2_waveformat4.txt", "DvbRcs2", "QPSK_1_TO_2", 536);
print_test_case ("rcs2_waveformat5.txt", "DvbRcs2", "QPSK_2_TO_3", 536);
print_test_case ("rcs2_waveformat6.txt", "DvbRcs2", "QPSK_3_TO_4", 536);
print_test_case ("rcs2_waveformat7.txt", "DvbRcs2", "QPSK_5_TO_6", 536);
print_test_case ("rcs2_waveformat8.txt", "DvbRcs2", "8PSK_2_TO_3", 536);
print_test_case ("rcs2_waveformat9.txt", "DvbRcs2", "8PSK_3_TO_4", 536);
print_test_case ("rcs2_waveformat10.txt", "DvbRcs2", "8PSK_5_TO_6", 536);
print_test_case ("rcs2_waveformat11.txt", "DvbRcs2", "16QAM_3_TO_4", 536);
print_test_case ("rcs2_waveformat12.txt", "DvbRcs2", "16QAM_5_TO_6", 536);
print_test_case ("rcs2_waveformat13.txt", "DvbRcs2", "QPSK_1_TO_3", 1616);
print_test_case ("rcs2_waveformat14.txt", "DvbRcs2", "QPSK_1_TO_2", 1616);
print_test_case ("rcs2_waveformat15.txt", "DvbRcs2", "QPSK_2_TO_3", 1616);
print_test_case ("rcs2_waveformat16.txt", "DvbRcs2", "QPSK_3_TO_4", 1616);
print_test_case ("rcs2_waveformat17.txt", "DvbRcs2", "QPSK_5_TO_6", 1616);
print_test_case ("rcs2_waveformat18.txt", "DvbRcs2", "8PSK_2_TO_3", 1616);
print_test_case ("rcs2_waveformat19.txt", "DvbRcs2", "8PSK_3_TO_4", 1616);
print_test_case ("rcs2_waveformat20.txt", "DvbRcs2", "8PSK_5_TO_6", 1616);
print_test_case ("rcs2_waveformat21.txt", "DvbRcs2", "16QAM_3_TO_4", 1616);
print_test_case ("rcs2_waveformat22.txt", "DvbRcs2", "16QAM_5_TO_6", 1616);

### DVB-S2 test cases ###

printf ("Ptr<SatLinkResultsDvbS2> linkResultsDvbS2 ");
printf ("= CreateObject<SatLinkResultsDvbS2> ();\n");
printf ("linkResultsDvbS2->Initialize ();\n");
printf ("\n");

print_test_case ("s2_16apsk_2_to_3.txt", "DvbS2", "16APSK_2_TO_3", 0);
print_test_case ("s2_16apsk_3_to_4.txt", "DvbS2", "16APSK_3_TO_4", 0);
print_test_case ("s2_16apsk_4_to_5.txt", "DvbS2", "16APSK_4_TO_5", 0);
print_test_case ("s2_16apsk_5_to_6.txt", "DvbS2", "16APSK_5_TO_6", 0);
print_test_case ("s2_16apsk_8_to_9.txt", "DvbS2", "16APSK_8_TO_9", 0);
print_test_case ("s2_16apsk_9_to_10.txt", "DvbS2", "16APSK_9_TO_10", 0);
print_test_case ("s2_32apsk_3_to_4.txt", "DvbS2", "32APSK_3_TO_4", 0);
print_test_case ("s2_32apsk_4_to_5.txt", "DvbS2", "32APSK_4_TO_5", 0);
print_test_case ("s2_32apsk_5_to_6.txt", "DvbS2", "32APSK_5_TO_6", 0);
print_test_case ("s2_32apsk_8_to_9.txt", "DvbS2", "32APSK_8_TO_9", 0);
print_test_case ("s2_8psk_2_to_3.txt", "DvbS2", "8PSK_2_TO_3", 0);
print_test_case ("s2_8psk_3_to_4.txt", "DvbS2", "8PSK_3_TO_4", 0);
print_test_case ("s2_8psk_3_to_5.txt", "DvbS2", "8PSK_3_TO_5", 0);
print_test_case ("s2_8psk_5_to_6.txt", "DvbS2", "8PSK_5_TO_6", 0);
print_test_case ("s2_8psk_8_to_9.txt", "DvbS2", "8PSK_8_TO_9", 0);
print_test_case ("s2_8psk_9_to_10.txt", "DvbS2", "8PSK_9_TO_10", 0);
print_test_case ("s2_qpsk_1_to_2.txt", "DvbS2", "QPSK_1_TO_2", 0);
print_test_case ("s2_qpsk_2_to_3.txt", "DvbS2", "QPSK_2_TO_3", 0);
print_test_case ("s2_qpsk_3_to_4.txt", "DvbS2", "QPSK_3_TO_4", 0);
print_test_case ("s2_qpsk_3_to_5.txt", "DvbS2", "QPSK_3_TO_5", 0);
print_test_case ("s2_qpsk_4_to_5.txt", "DvbS2", "QPSK_4_TO_5", 0);
print_test_case ("s2_qpsk_5_to_6.txt", "DvbS2", "QPSK_5_TO_6", 0);
print_test_case ("s2_qpsk_8_to_9.txt", "DvbS2", "QPSK_8_TO_9", 0);
print_test_case ("s2_qpsk_9_to_10.txt", "DvbS2", "QPSK_9_TO_10", 0);
