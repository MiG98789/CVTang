function [neighbourhood, edgeCosts, smoothness] = getGraph(Icosahedron, potentialNormals)

[W, H, ~] = size(potentialNormals);
totalSize = W*H;

[icoSize, ~] = size(Icosahedron);

neighbourhood = sparse(totalSize,totalSize);
edgeCosts = zeros(icoSize,totalSize);
smoothness = zeros(icoSize, icoSize);
lambda = 0.01;
sigma = 1;
epsilon = 0.01;

for row = 0:H-1
  for col = 0:W-1
    pixel = 1+ row*W + col;
    
    if row+1 < H
        tempIndex = 1+col+(row+1)*W;
        ico = Icosahedron(potentialNormals(pixel), :) - Icosahedron(potentialNormals(tempIndex), :);
        ico = sqrt(ico*ico');
        neighbourhood(pixel, tempIndex) = lambda * log(1 + ico/sigma); 
    end
    
    if row-1 >= 0
        tempIndex = 1+col+(row-1)*W;
        ico = Icosahedron(potentialNormals(pixel), :) - Icosahedron(potentialNormals(tempIndex), :);
        ico = sqrt(ico*ico');
        neighbourhood(pixel, tempIndex) = lambda * log(1 + ico/sigma); 
    end
    
    if col+1 < W
        tempIndex = 1+(col+1)+row*W;
        ico = Icosahedron(potentialNormals(pixel), :) - Icosahedron(potentialNormals(tempIndex), :);
        ico = sqrt(ico*ico');
        neighbourhood(pixel, tempIndex) = lambda * log(1 + ico/sigma); 
    end
    
    if col-1 >= 0
        tempIndex = 1+(col-1)+row*W;
        ico = Icosahedron(potentialNormals(pixel), :) - Icosahedron(potentialNormals(tempIndex), :);
        ico = sqrt(ico*ico');
        neighbourhood(pixel, tempIndex) = lambda * log(1 + ico/sigma); 
    end
    
    for i = 1:icoSize
        ico = Icosahedron(i, :) - Icosahedron(potentialNormals(pixel)+1, :);
        edgeCosts(i,pixel) = sqrt(ico*ico');
    end
  end
end

for i = 1:icoSize
    for j = 1:icoSize
        Sij = Icosahedron(i, :) - Icosahedron(j, :);
        Sij = sqrt(Sij*Sij');
        K = 1 + epsilon - exp(-(2-Sij)/sigma^2);
        smoothness(i, j) = lambda*K*Sij;
    end
end

end