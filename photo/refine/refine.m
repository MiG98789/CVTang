function  refinedNormal = refine(normalName)

singleSurfaceImg = imread(strcat(normalName, '.jpg'));
[initialH, initialW, ~] = size(singleSurfaceImg);

surfaceNormal = csvread(strcat(normalName, '.csv'));
surfaceNormal = reshape(surfaceNormal, initialH, initialW, 3);
[W, H, ~] = size(surfaceNormal);

Icosahedron = icosahedron(0.2);

potentialNormals = getPotentialNormals(Icosahedron, surfaceNormal);

[neighbourhood, edgeCosts, smoothness] = getGraph(Icosahedron, potentialNormals);

potentialNormals = potentialNormals - 1;
[labels, ~, ~] = GCMex(potentialNormals, single(edgeCosts), neighbourhood, single(smoothness), 1);
labels = labels + 1;

labels = reshape(labels, W, H);
refinedNormal = zeros(W, H, 3);

for i = 1:W
    for j = 1:H
        refinedNormal(i, j, :) = Icosahedron(labels(i,j), :);
    end
end

%refinedNormal = reshape(refinedNormal, initialW, initialH, 3);
%refinedNormal = rot90(refinedNormal, 3);

end
