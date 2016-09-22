% Copyright (c) 2014 Magister Solutions Ltd
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License version 2 as
% published by the Free Software Foundation;
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
%
% Author: Jani Puttonen <jani.puttonen@magister.fi>
%

clear all;
close all;

% ---------------------------------------------------------
% This script can be used to create a external fading index
% files with UT positions from the .mat file located under
% satellite/data/ext-fadingfiles/input folders. Each fading
% file comes with a position, which can be enforced by the
% simulator with list position allocator and external fading
% files in POSITION_MODE. Remember to change the folders to 
% the ones you are interested.
% ---------------------------------------------------------

beamId = 39;

dir = '$NS3_ROOT/contrib/satellite/data/ext-fadingtraces/input/';
subDir = ['BeamId-' int2str(beamId) '_256_Terminals/'];
locFileName = ['terminal_locations_beamId_' int2str(beamId) '.mat'];

outputFwdDwnFileName = ['BeamId-' int2str(beamId) '_256_UT_fading_fwddwn_trace_index.txt'];
outputRtnUpFileName = ['BeamId-' int2str(beamId) '_256_UT_fading_rtnup_trace_index.txt'];
outputPosFileName = ['BeamId-' int2str(beamId) '_256_UT_Positions.txt'];

loc = load([dir subDir locFileName]);
terminals = length(loc.terminals.pos.lat);

earthRadius = 6378137;

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
