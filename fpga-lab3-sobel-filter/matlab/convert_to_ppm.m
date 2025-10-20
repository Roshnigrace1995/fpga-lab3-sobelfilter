% convert_to_ppm.m
% Converts lena_gray.png into lena_input.ppm for Vivado HLS Sobel Lab

img = imread('../images/lena_gray.png');   % Read the image
if size(img,3) == 3
    img = rgb2gray(img);                   % Convert to grayscale if needed
end
img = imresize(img, [512 512]);            % Ensure 512x512 pixels
 imwrite(img, '../images/lena_input.pgm', 'Encoding','raw');   % PGM=P5
disp('✅ lena_input.ppm created successfully.');
