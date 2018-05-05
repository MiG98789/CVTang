function reconstructedSurf = getReconstructedSurface(normalName, dataNumber)

singleSurfaceImg = imread(strcat('NL02.jpg'));
[initialH, initialW, ~] = size(singleSurfaceImg);

surfaceNormal = csvread(strcat('NRL.csv'));

surfaceNormal = reshape(surfaceNormal, initialH, initialW, 3);
[M, N, ~] = size(surfaceNormal);

slant = zeros(M, N);
tilt = zeros(M, N);
for i = 1:M
    for j = 1:N
        slant(i, j) = surfaceNormal(i, j, 2);
        tilt(i, j)  = surfaceNormal(i, j, 3);
    end
    
end
reconstructedSurf = shapeletsurf(slant, tilt, 6, 1, 2);
rotate(surf(reconstructedSurf), [0 0 1], 270);
end
