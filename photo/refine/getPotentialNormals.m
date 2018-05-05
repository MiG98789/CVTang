function potentialNormals = getPotentialNormals(Icosahedron, surfaceNormal)
[W, H, ~] = size(surfaceNormal);
potentialNormals = zeros(W, H);

for i = 1:W
    for j = 1:H
        d = (Icosahedron(:,1)-surfaceNormal(i,j,1)).^2 + ...
            (Icosahedron(:,2)-surfaceNormal(i,j,2)).^2 + ...
            (Icosahedron(:,3)-surfaceNormal(i,j,3)).^2;
        [~, index] = min(d);
        potentialNormals(i, j) = index;
    end
end

potentialNormals = reshape(potentialNormals, 1, [])';
end