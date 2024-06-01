#include "lodepng.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct point{
    int x, y;
}pt;

char* load_png_file(const char *filename, int *width, int *height) {
	unsigned char *image = NULL;
	int error = lodepng_decode32_file(&image, width, height, filename);
	if (error) {
		printf("error %u: %s\n", error, lodepng_error_text(error));
		return NULL;
	}
	return (image);
}

void write_png_file(const char *filename, const unsigned char *image, unsigned width, unsigned height) {
    unsigned char* png;
    size_t size;
    int error = lodepng_encode32(&png, &size, image, width, height);
    if (!error) {
        lodepng_save_file(png, size, filename);
    }
    free(png);
}

void colour(unsigned char* image, int widht, int height, int eps)
{
    for(int j = 1; j < height - 1; j++) {
        for(int i = 1; i < widht - 1; i++) {
		int colour1 = rand() % (255 - eps * 2) + eps * 2;
   		int colour2 = rand() % (255 - eps * 2) + eps * 2;
        int colour3 = rand() % (255 - eps * 2) + eps * 2;
        if(image[4 * (j * widht + i)] < eps || image[4 * (j * widht + i)] > 255-(eps*2)) {
		    int dx[] = {-1, 0, 1, 0};
		    int dy[] = {0, 1, 0, -1};
            pt* stack = malloc(widht * height * 4 * sizeof(pt));
		    long top = 0;
		    stack[top++] = (pt){i, j};
		    while(top > 0) {
		        pt pix = stack[--top];
		        if(pix.x < 0 || pix.x >= widht || pix.y < 0 || pix.y >= height) continue;
		        int ind = (pix.y * widht + pix.x) * 4;
		        if(image[ind] > eps) continue;
		        image[ind] = colour1;
		        image[ind + 1] = colour2;
		        image[ind + 2] = colour3;
		        for(int i = 0; i < 4; i++) {
		            int newx = pix.x + dx[i];
		            int newy = pix.y + dy[i];
		            if(newx > 0 && newx < widht && newy > 0 && newy < height) {
		                stack[top++] = (pt){newx, newy};
		            }
		    	}
		    }
		    free(stack);
		}
            }
        }
    }



void Sobel_filter(unsigned char *image, int widht, int height) {

    int kernelx[3][3] = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
    int kernely[3][3] = {{1, 2, 1}, {0, 0, 0}, {-1, -2, -1}};
    int x, y;
    unsigned char *temp_image = malloc(widht * height * 4 * sizeof(unsigned char));
    for(y = 1; y < height - 1; y++) {
        for(x = 1; x < widht - 1; x++) {
            int dx = 0;
            int dy = 0;
            for(int i = -1; i <= 1; i++) {
                for(int j = -1; j <= 1; j++) {
                    int ind;
                    ind = ((y + j) * widht + (i + x)) * 4;
                    int rgb = (image[ind] + image[ind + 1] + image[ind + 2]) / 3;
                    dx += kernelx[j + 1][i + 1] * rgb;
                    dy += kernely[j + 1][i + 1] * rgb;
                }
            }
            int grad;
            grad = sqrt(dx * dx + dy * dy);
            if(grad > 255) {
                grad = 255;
            }
            int res = (y * widht + x) * 4;
            temp_image[res] = (unsigned char)grad;
            temp_image[res + 1] = (unsigned char)grad;
            temp_image[res + 2] = (unsigned char)grad;
            temp_image[res + 3] = image[res + 3];
        }
    }
    for(int k = 0; k < widht * height * 4; k++) {
        image[k] = temp_image[k];
    }
    free(temp_image);
    return;
}

unsigned char *make_gray(unsigned char* image, unsigned int widht, unsigned int height)
{
    unsigned char *temp_image = image;
    unsigned char maxR = 0, maxG = 0, maxB = 0;
    unsigned char r, g, b, a;
    for(int i = 0; i < 4*height*widht; i+=4){
            r = temp_image[i];
            g = temp_image[i + 1];
            b = temp_image[i + 2];
            a = temp_image[i + 3];
            maxR = fmax(r, maxR);
            maxG = fmax(g, maxG);
            maxB = fmax(b, maxB);
        }
    temp_image = image;
    unsigned char newr, newg, newb, newa;
    for(int i = 0; i < 4*widht*height; i += 4){
            newr = temp_image[i];
			newg = temp_image [i + 1];
			newb = temp_image[i + 2];
			newa = temp_image[i + 3];
			unsigned char gray = 255.0 * ((float)newr / maxR + (float)newg / maxG + (float)newb / maxB) / 3;
			temp_image[i] = temp_image[i + 1] = temp_image [i + 2] = (unsigned char)gray;
        }
    return temp_image;
}


int main()
{
	int w = 0, h = 0;
	int k = 0;
	double epsilon = 25.0;
	unsigned char *filename = "skull.png";
	unsigned char *output = "skull-result.png";
	unsigned char *image = load_png_file(filename, &w, &h);
	unsigned char *nimage = make_gray(image, w, h);
	Sobel_filter(nimage, w, h);
    colour(nimage,w,h,epsilon);
	write_png_file(output, nimage, w, h);
	free(image);
	free(nimage);
	return 0;
}
