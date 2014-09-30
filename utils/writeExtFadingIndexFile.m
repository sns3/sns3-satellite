clear all;
close all;

% ---------------------------------------------------------
% Author: Jani Puttonen
% Date: 30.9.2014
% This script can be used to create a external fading index
% files with UT positions from the .mat file located under
% satellite/data/ext-fadingfiles/input folders. Each fading
% file comes with a position, which can be enforced by the
% simulator with list position allocator and external fading
% files in POSITION_MODE. Remember to change the folders to 
% the ones you are interested.
% ---------------------------------------------------------

beamId = 51;

dir = '$NS3_ROOT/src/satellite/data/ext-fadingtraces/input/';
subDir = ['BeamId-' int2str(beamId) '_256_Terminals/'];
locFileName = ['terminal_locations_beamId_' int2str(beamId) '.mat'];

outputFwdDwnFileName = ['BeamId-' int2str(beamId) '_256_UT_fading_fwddwn_trace_index.txt'];
outputRtnUpFileName = ['BeamId-' int2str(beamId) '_256_UT_fading_rtnup_trace_index.txt'];
outputPosFileName = ['BeamId-' int2str(beamId) '_256_UT_Positions.txt'];

loc = load([dir subDir locFileName]);
terminals = length(loc.terminals.pos.lat);

earthRadius = 6371000;

lat = loc.terminals.pos.lat .* 180/pi;
lon = loc.terminals.pos.lon .* 180/pi;
alt = (loc.terminals.pos.r .- earthRadius);

fwdDwnId = fopen(outputFwdDwnFileName, 'w');
rtnUpId = fopen(outputRtnUpFileName, 'w');
posId = fopen(outputPosFileName, 'w');

for i = 1:terminals;
  pos = [num2str(lat(i)) ' ' num2str(lon(i)) ' ' num2str(alt(i))];
  ind_fwddwn_row = [int2str(i) ' ' subDir 'term_ID' int2str(i) '_fading_fwddwn.dat ' pos];
  ind_rtnup_row = [int2str(i) ' ' subDir 'term_ID' int2str(i) '_fading_rtnup.dat ' pos];

  fprintf(fwdDwnId, '%s\n', ind_fwddwn_row);
  fprintf(rtnUpId, '%s\n', ind_rtnup_row);
  fprintf(posId, '%s\n', pos);
end

fclose('all');
