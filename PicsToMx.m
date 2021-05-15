image_files=dir(fullfile('./Img/','*.bmp'));
pixel_matrix = zeros(160,128,3,52, 'uint8');

for i=1:length(image_files)
   pixel_matrix(:,:,:,i) = imread(image_files(i).name);
end
%%
movie_fid = fopen('LineRabbit.mmov', 'a');
pixel = 0;
for i = 1 : 52
    for y = 160:-1:1
        for x = 1 : 128
            pixel = 0;
            for color = 1:3
                tri = double(pixel_matrix(y,x,color,i)) / 255.0 * 63.0;
                tri = bitshift(uint32(tri), (8 * (3 - color) + 2));
                pixel = bitor(pixel, tri);
            end
         fwrite(movie_fid, pixel,'uint32');
        end
    end
end
1+1
fclose(movie_fid);