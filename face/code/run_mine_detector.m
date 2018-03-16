% Starter code prepared by James Hays for CS 143, Brown University
% This function returns detections on all of the images in a given path.
% You will want to use non-maximum suppression on your detections or your
% performance will be poor (the evaluation counts a duplicate detection as
% wrong). The non-maximum suppression is done on a per-image basis. The
% starter code includes a call to a provided non-max suppression function.
function [hard_negatives] = .... 
    run_mine_detector(non_face_scn_path, w, b, feature_params)
% 'test_scn_path' is a string. This directory contains images which may or
%    may not have faces in them. This function should work for the MIT+CMU
%    test set but also for any other images (e.g. class photos)
% 'w' and 'b' are the linear classifier parameters
% 'feature_params' is a struct, with fields
%   feature_params.template_size (probably 36), the number of pixels
%      spanned by each train / test template and
%   feature_params.hog_cell_size (default 6), the number of pixels in each
%      HoG cell. template size should be evenly divisible by hog_cell_size.
%      Smaller HoG cell sizes tend to work better, but they make things
%      slower because the feature dimensionality increases and more
%      importantly the step size of the classifier decreases at test time.

% 'bboxes' is Nx4. N is the number of detections. bboxes(i,:) is
%   [x_min, y_min, x_max, y_max] for detection i. 
%   Remember 'y' is dimension 1 in Matlab!
% 'confidences' is Nx1. confidences(i) is the real valued confidence of
%   detection i.
% 'image_ids' is an Nx1 cell array. image_ids{i} is the image file name
%   for detection i. (not the full path, just 'albert.jpg')

% The placeholder version of this code will return random bounding boxes in
% each test image. It will even do non-maximum suppression on the random
% bounding boxes to give you an example of how to call the function.

% Your actual code should convert each test image to HoG feature space with
% a _single_ call to vl_hog for each scale. Then step over the HoG cells,
% taking groups of cells that are the same size as your learned template,
% and classifying them. If the classification is above some confidence,
% keep the detection and then pass all the detections for an image to
% non-maximum suppression. For your initial debugging, you can operate only
% at a single scale and you can skip calling non-maximum suppression.

non_face_scenes = dir( fullfile( non_face_scn_path, '*.jpg' ));

c_size = feature_params.hog_cell_size;
t_size = feature_params.template_size;
window = (t_size/c_size);
scale = [0.01, 0.02, 0.05:0.05:1];

D = window^2 * 31;
hard_negatives = zeros(0, D);

for i = 1:length(non_face_scenes)
      
    fprintf('Detecting non-faces in %s\n', non_face_scenes(i).name)
    img = imread( fullfile( non_face_scn_path, non_face_scenes(i).name ));
    img = single(img)/255;
    if(size(img,3) > 1)
        img = rgb2gray(img);
    end

    for s = scale
        scaled_img = imresize(img, s);

        hog = vl_hog(single(scaled_img), c_size);

        %Get how many windows are in the image
        countx = floor((size(scaled_img, 2) - t_size) /c_size) + 1;
        county = floor((size(scaled_img, 1) - t_size) /c_size) + 1;

        scaled_features = zeros(countx*county, D);

        for stepx = 1:countx
            for stepy = 1:county
                %Get hog of a window and append it to features
                sub_hog = hog(stepy:stepy+window-1, stepx:stepx+window-1, :);
                scaled_features((stepx-1)*county+stepy, :) = reshape(sub_hog, 1, D);
            end
        end

        %Use score output as filtering for all entries
        scores = scaled_features * w + b;
        filter = find(scores > 0.8);

        %Extract false positive features and append to negative examples
        hard_negatives = [hard_negatives; scaled_features(filter, :)];
    end
end
