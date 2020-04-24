% Copyright (c) 2016 Magister Solutions Ltd
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

% This Matlab script can be used to plot different kind of figures
% from the antenna gain patterns coming with the satellite module.
% There are in total antenna gain patterns from 72 spot beams from
% which we form a "max" antenna gain pattern, where each latitude-
% longitude coordinate has the antenna gain of the antenna beam
% providing the maximum antenna gain. The same information is used
% to build dominance map presenting which beam is the best in a 
% certain latitude-longitude coordinate. The script also reads the 
% satellite system configuration file enabling it to plot beam
% ids, user and feeder link frequency ids, and GW ids.
% 
% Requirements
% - Matlab
% - Mapping toolbox for Matlab
%
% Please run the script in data folder.

close all; clear all;

minGain = 48;
confFileName='Scenario72Conf.txt';
conf = importdata(confFileName);

gwPosFileName = 'Scenario72GwPos.txt';
gwPos = importdata(gwPosFileName);

beamCenters = [];
allData = [];
BEAMS = 72;

singleBeamId = 1;
singleBeamPattern = [];

for i = 1:BEAMS
    res = [];
    fileName = ['antennapatterns/SatAntennaGain72Beams_' int2str(i), '.txt']
    res = load(fileName);
    
    % Store x, y coordinates
    if ( i == 1 )
        latitude = res(:,1);
        longitude = res(:,2);
    end

    % Find beam centers
    ind = find(res(:,3)==max(res(:,3)));
    if (i == 1)
      beamCenters = [res(ind(1,1),1) res(ind(1,1),2)]; 
    else
      beamCenters = [beamCenters; [res(ind(1,1),1) res(ind(1,1),2)]];
    end

    % store antenna gain patterns
    allData = [ allData res(:,3) ];
end

singleBeamPattern = allData(:,singleBeamId);

[G I] = max(allData');
G = G';
I = I';

UF = zeros(size(G));
FF = zeros(size(G));
GW = zeros(size(G));
for i = 1:length(I);
  if (G(i) > minGain)    
    UF(i) = conf(I(i),2);
    FF(i) = conf(I(i),4);
    GW(i) = conf(I(i),3);
  else
    UF(i) = 0;
    FF(i) = 0;
    GW(i) = 0;
    I(i) = 0;
  end
end

UF = UF';
FF = FF';
GW = GW';

lat = unique(latitude);
lon = unique(longitude);

% Make lat and lon grids
[LatGrid, LonGrid] = meshgrid(lat, lon);
LatGrid2 = fliplr(LatGrid');
LonGrid2 = LonGrid';

l = latitude(1,1);
for i = 1:length(latitude);
    if (latitude(i) ~= l)
        repetition = i-1;
        break;
    end
end

% Reshape the gain pattern 
s = length(G);

shapedG = reshape(G, repetition, s/repetition)';
shapedI = reshape(I, repetition, s/repetition)';
shapedFF = reshape(FF, repetition, s/repetition)';
shapedUF = reshape(UF, repetition, s/repetition)';
shapedGW = reshape(GW, repetition, s/repetition)';
shapedSB = reshape(singleBeamPattern, repetition, s/repetition)';

set(gcf,'renderer','opengl');

%% Plot antenna gain patterns with map
%% -----------------------------------
worldmap world;
load coastlines;

%load coast;
%plotm(lat, long);
%[latcells, loncells] = polysplit(lat, long);
%numel(latcells);

h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
plotm(coastlat,coastlon,'Color','black', 'LineWidth', 1.5);

%geoshow('landareas.shp', 'FaceColor', [0.15 0.5 0.15]);
%geoshow('landareas.shp', 'FaceColor', 'white', 'FaceAlpha', 0.3);
%h = surfacem(LatGrid2, LonGrid2, shapedG, 'FaceAlpha', 0.8);

h = surfacem(LatGrid2, LonGrid2, shapedG);
uistack(h, 'bottom');

% Plot beam ids
for j = 1:BEAMS
  textm(beamCenters(j,1), beamCenters(j,2)-1, int2str(conf(j,1)), 'FontSize', 10, 'Color', 'black');
end

caxis([48 53]);
cmap = colormap(jet);
cmap(1,:) = [1 1 1];
colormap(cmap);
colorbar;

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Antenna gain patterns of the 72 beam reference system');
print('SNS3-full72-gain-map','-dpng');


%% Plot antenna gain patterns with GW positions
%% --------------------------------------------
worldmap world;
load coastlines;

h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
plotm(coastlat,coastlon,'Color','black', 'LineWidth', 1.5);

h = surfacem(LatGrid2, LonGrid2, shapedG);
uistack(h, 'bottom');

% Plot beam ids
for j = 1:length(gwPos)
  textm(gwPos(j,1), gwPos(j,2), ['x GW: ' int2str(j)], 'FontSize', 10, 'Color', 'black');
end

caxis([48 53]);
cmap = colormap(jet);
cmap(1,:) = [1 1 1];
colormap(cmap);
colorbar;

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Antenna gain patterns of the 72 beam reference system with GW positions');
print('SNS3-full72-gain-map-gwpos','-dpng');

%% Plot gain pattern only
%% ----------------------
figure;
worldmap world;
h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
h = surfacem(LatGrid2, LonGrid2, shapedG);

colormap jet;
colorbar;

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Antenna gain patterns of 72 beam reference system');
print('SNS3-full72-gain-only','-dpng');

%% Plot one beam gain pattern only
%% -------------------------------
figure;
worldmap world;
h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
h = surfacem(LatGrid2, LonGrid2, shapedSB);

colormap jet;
colorbar;

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title(['Antenna gain pattern of beam: ' int2str(singleBeamId)]);
print(['SNS3-full72-gain-beam-' int2str(singleBeamId)],'-dpng');


%% Plot dominance map - beam id
%% ----------------------------

figure;
worldmap world;
load coastlines;
h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
plotm(coastlat,coastlon,'Color','black', 'LineWidth', 1.5);

h = surfacem(LatGrid2, LonGrid2, shapedI);
uistack(h, 'bottom');

cmap = colormap(jet);
cmap(1,:) = [1 1 1];
colormap(cmap);
colorbar;

for j = 1:BEAMS
  textm(beamCenters(j,1), beamCenters(j,2)-1, int2str(conf(j,1)), 'FontSize', 10, 'Color', 'black');
end

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Dominance map of 72 beam reference system (beam id)');
print('SNS3-full72-dominance-map-beamids','-dpng');

%% Plot dominance map - UF
%% -----------------------

figure;
worldmap world;
load coastlines;
h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
plotm(coastlat,coastlon,'Color','black', 'LineWidth', 1.5);

h = surfacem(LatGrid2, LonGrid2, shapedUF);
uistack(h, 'bottom');

cmap = colormap(jet);
cmap(1,:) = [1 1 1];
colormap(cmap);
colorbar;

for j = 1:BEAMS
  textm(beamCenters(j,1), beamCenters(j,2)-1, int2str(conf(j,2)), 'FontSize', 10, 'Color', 'black');
end

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Dominance map of 72 beam reference system (user frequency id)');
print('SNS3-full72-dominance-map-userfreq','-dpng');


%% Plot dominance map - FF
%% -----------------------

figure;
worldmap world;
load coastlines;
h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
plotm(coastlat,coastlon,'Color','black', 'LineWidth', 1.5);

h = surfacem(LatGrid2, LonGrid2, shapedFF);
uistack(h, 'bottom');

cmap = colormap(jet);
cmap(1,:) = [1 1 1];
colormap(cmap);
colorbar;

for j = 1:BEAMS
  textm(beamCenters(j,1), beamCenters(j,2)-1, int2str(conf(j,4)), 'FontSize', 10, 'Color', 'black');
end

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Dominance map of 72 beam reference system (feeder frequency id)');
print('SNS3-full72-dominance-map-feederfreq','-dpng');

%% Plot dominance map - GW
%% -----------------------

figure;
worldmap world;
load coastlines;
h = worldmap('Europe'); hold on;
getm(h,'MapProjection');
plotm(coastlat,coastlon,'Color','black', 'LineWidth', 1.5);

h = surfacem(LatGrid2, LonGrid2, shapedGW);
uistack(h, 'bottom');

cmap = colormap(jet);
cmap(1,:) = [1 1 1];
colormap(cmap);
colorbar;

for j = 1:BEAMS
  textm(beamCenters(j,1), beamCenters(j,2)-1, int2str(conf(j,3)), 'FontSize', 10, 'Color', 'black');
end

xlabel('Longitude [deg]');
ylabel('Latitude [deg]');
title('Dominance map of 72 beam reference system (GW id)');
print('SNS3-full72-dominance-map-gwid','-dpng');

