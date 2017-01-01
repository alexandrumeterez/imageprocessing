#include <stdio.h>
#include "bmp_header.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

int flipCount = 1;

int MULT(unsigned char x, int y)
{
	return (int)(x)*y;
}

void swap(unsigned char *x, unsigned char *y)
{
	(*x) ^= (*y);
	(*y) ^= (*x);
	(*x) ^= (*y);
}

void flipImage(pixel ***image, bmpInfoHeader infoHeader)
{

	pixel temp;
	int i, j;
	for(i=0; i<infoHeader.width; i++)
		for(j=0; j<infoHeader.height/2; j++)
		{
			temp = (*image)[j][i];
			(*image)[j][i]=(*image)[infoHeader.height - 1 - j][i];
			(*image)[infoHeader.height - 1 - j][i] = temp;
		}
	if(flipCount == 1)
		flipCount = 0;
	else flipCount = 1;
	printf("Flip count: %d\n", flipCount);
}

pixel **LoadImage(char *FileName, bmpInfoHeader *infoHeader, bmpFileHeader *fileHeader)
{
	FILE *file;
	file = fopen(FileName, "rb");
	fread(fileHeader, sizeof(bmpFileHeader), 1, file);

	fread(infoHeader, 40, 1, file);

	fseek(file, fileHeader->imageDataOffset, SEEK_SET);
	pixel **img;
	img = malloc(infoHeader->height * sizeof(pixel*));
	int i, j;
	for(i=0; i < infoHeader->height; i++)
		img[i] = malloc(infoHeader->width * sizeof(pixel));

	printf("%d %d\n", infoHeader->height, infoHeader->width);

	for(i=0; i< infoHeader->height; i++)
		for(j=0; j< infoHeader->width; j++)
		{
			fread(&img[i][j], sizeof(unsigned char), sizeof(pixel), file);
			//fseek(file, infoHeader->width*3+4-infoHeader->width*3%4, SEEK_CUR);
			swap(&img[i][j].r, &img[i][j].b);
		}
	flipImage(&img, *infoHeader);
	fclose(file);
	return img;
}

void writeImage(char *FileName, bmpInfoHeader infoHeader, bmpFileHeader fileHeader, pixel **image)
{
	FILE *file;
	file = fopen(FileName, "w");
	int i, j;
	//flipImage(&image, infoHeader);
	fwrite(&fileHeader, sizeof(bmpFileHeader), 1, file);
	fwrite(&infoHeader, 40, 1, file);
	fseek(file, fileHeader.imageDataOffset, SEEK_SET);
	for(i=0; i<infoHeader.height;i++)
		for(j=0; j<infoHeader.width;j++)
			swap(&image[i][j].r, &image[i][j].b);
	for(i=0; i< infoHeader.height; i++)
		for(j=0; j< infoHeader.width; j++)
			fwrite(&image[i][j], sizeof(unsigned char), sizeof(pixel), file);
	fclose(file);
	printf("Exported to: %s\n", FileName);

}

char *extractFilename(char *filename)
{
	//printf("%s\n", filename);
	char *newFilename = NULL;
	newFilename = malloc(strlen(filename)+20);//DE MODIFICAT!!!!!!!!!!!!!!!!!
	strncpy(newFilename, filename, (strlen(filename)-4));
	newFilename[strlen(filename) - 4] = 0;
	return newFilename;
}

pixel BWpixel(pixel p)
{
	p.r = (p.r + p.g + p.b)/3;
	p.g = p.r;
	p.b = p.r;
	return p;
}

void RGBtoBW(char *imageName, bmpInfoHeader infoHeader, bmpFileHeader fileHeader)
{
	char *BWimageName;
	printf("%s\n", imageName);
	pixel **image;
	image = LoadImage(imageName, &infoHeader, &fileHeader);
	//printf("%d\n", sizeof(BWimageName));
	BWimageName = extractFilename(imageName);
	strcat(BWimageName, "_black_white.bmp");

	//printf("%s\n", BWimageName);

	//file = fopen(BWimageName, "w");

	int i, j;
	for(i=0; i<infoHeader.height; i++)
		for(j=0; j<infoHeader.width; j++)
			image[i][j] = BWpixel(image[i][j]);
	flipImage(&image, infoHeader);
	writeImage(BWimageName, infoHeader, fileHeader, image);
	//printf("Exported to: %s\n", BWimageName);


}

int checkTemp(int temp)
{
	if(temp > 255)
		return 255;
	else if(temp < 0)
		return 0;
	else return temp;
}

void applyFilters(char *imageName, bmpInfoHeader infoHeader, bmpFileHeader fileHeader)
{


	int F1[3][3] = {
		{-1, -1, -1},
		{-1, 8, -1},
		{-1, -1, -1}
	};

	int F2[3][3] = {
		{0, 1, 0},
		{1, -4, 1},
		{0, 1, 0}
	};

	int F3[3][3] = {
		{1, 0, -1},
		{0, 0, 0},
		{-1, 0, 1}
	};
	pixel **image;
	char *bwName;
	bwName = extractFilename(imageName);
	strcat(bwName, "_black_white.bmp");
	image = LoadImage(bwName, &infoHeader, &fileHeader);
	char *filter1Name, *filter2Name, *filter3Name;
	filter1Name = extractFilename(imageName);
	strcat(filter1Name, "_f1.bmp");
	filter2Name = extractFilename(imageName);
	strcat(filter2Name, "_f2.bmp");
	filter3Name = extractFilename(imageName);
	strcat(filter3Name, "_f3.bmp");

	int i, j, k, l;
	int temp;
	pixel **copyImage;
	//creez o matrice auxiliara bordata cu 0uri pe toate laturile
	copyImage = calloc((infoHeader.height + 2), sizeof(pixel*));
	for(i=0; i < infoHeader.height + 2; i++)
		copyImage[i] = calloc((infoHeader.width + 2), sizeof(pixel));

	//flipImage(&image, infoHeader);
	//copiez in matricea auxiliara matricea originala, pastrand bordarea
	for(i=1; i < infoHeader.height + 1; i++)
		for(j=1; j < infoHeader.width + 1; j++)
			copyImage[i][j] = image[i-1][j-1];

	//aplic filtru lu peste
	for(i=0; i<infoHeader.height ;i++)
		for(j=0;j<infoHeader.width;j++)
		{

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].r, F1[k][l]);
			temp = checkTemp(temp);
			image[i][j].r = temp;

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].g, F1[k][l]);
			temp = checkTemp(temp);
			image[i][j].g = temp;

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].b, F1[k][l]);
			temp = checkTemp(temp);
			image[i][j].b = temp;
		}

	flipImage(&image, infoHeader);
	writeImage(filter1Name, infoHeader, fileHeader, image);

	flipImage(&image, infoHeader);
	for(i=0; i<infoHeader.height ;i++)
		for(j=0;j<infoHeader.width;j++)
		{

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].r, F2[k][l]);
			temp = checkTemp(temp);
			image[i][j].r = temp;

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].g, F2[k][l]);
			temp = checkTemp(temp);
			image[i][j].g = temp;

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].b, F2[k][l]);
			temp = checkTemp(temp);
			image[i][j].b = temp;
		}
	flipImage(&image, infoHeader);
	writeImage(filter2Name, infoHeader, fileHeader, image);

	flipImage(&image, infoHeader);
	for(i=0; i<infoHeader.height ;i++)
		for(j=0;j<infoHeader.width;j++)
		{

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].r, F3[k][l]);
			temp = checkTemp(temp);
			image[i][j].r = temp;

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].g, F3[k][l]);
			temp = checkTemp(temp);
			image[i][j].g = temp;

			temp = 0;
			for(k=0; k<3; k++)
				for(l=0; l<3; l++)
					temp += MULT(copyImage[i+k][j+l].b, F3[k][l]);
			temp = checkTemp(temp);
			image[i][j].b = temp;
		}
	flipImage(&image, infoHeader);
	writeImage(filter3Name, infoHeader, fileHeader, image);
}

unsigned char myabs(unsigned char a, unsigned char b)
{
	int aa = (int)a;
	int bb = (int)b;

	if(aa>bb)
		return (unsigned char)(aa-bb);
	else return (unsigned char)(bb-aa);

}

Bool checkPixelBorder(int i, int j, int height, int width, unsigned int **zoneMatrix)
{
	unsigned int zonaCur = zoneMatrix[i][j];
	if(i==0 || j==0 || i == height - 1 || j == width - 1)
		return true;
	if(i > 0)
		if(zonaCur != zoneMatrix[i-1][j])
			return true;
	if(i < height - 1)
		if(zonaCur != zoneMatrix[i+1][j])
			return true;
	if(j > 0)
		if(zonaCur != zoneMatrix[i][j-1])
			return true;
	if(j < width - 1)
		if(zonaCur != zoneMatrix[i][j+1])
			return true;

	return false;
}

void compressImage(pixel **image, bmpInfoHeader infoHeader, bmpFileHeader fileHeader, int threshold)
{

	FILE *fileWrite;
	fileWrite = fopen("compressed.bin", "wb");

	fwrite(&fileHeader, sizeof(bmpFileHeader), 1, fileWrite);
	fwrite(&infoHeader, 40, 1, fileWrite);
	fseek(fileWrite, fileHeader.imageDataOffset, SEEK_SET);

	int zona = 0;
	unsigned int **zoneMatrix;
	int i, j;
	zoneMatrix = calloc(infoHeader.height, sizeof(unsigned int*));
	for(i=0; i < infoHeader.height; i++)
		zoneMatrix[i] = calloc(infoHeader.width, sizeof(unsigned int));

	//pentru coada
	int prim=0;
	int **coada;
	coada = calloc(2, sizeof(int*));
	coada[0] = calloc(infoHeader.biSizeImage, sizeof(int));
	coada[1] = calloc(infoHeader.biSizeImage, sizeof(int));

	printf("%d, %d %d\n", infoHeader.biSizeImage, infoHeader.width, infoHeader.height);
	//printf("%ld\n", sizeof(coada)/sizeof(int*));
	int k;
	pixel pixelSus, pixelJos, pixelStanga, pixelDreapta, pixelStart;
	for(i=0; i<infoHeader.height; i++)
		for(j=0; j<infoHeader.width;j++)
		{
			//printf("%d\n", threshold);
			if(zoneMatrix[i][j] == 0)
			{
				prim=0;
				zona++;

			//daca nu e in nicio zona il pun i ncoada si il marchez
				coada[0][prim] = i;
				coada[1][prim] = j;
				prim++;
				zoneMatrix[i][j] = zona;
				pixelStart = image[coada[0][0]][coada[1][0]];
				for(k=0; k<prim; k++)
				{
				//	printf("zona: %d\n", zona);
					//testez vecinii elementului

					//verific pixel sus
					if(coada[0][k]>0 && zoneMatrix[coada[0][k] - 1][coada[1][k]]==0)
					{
						pixelSus = image[coada[0][k]-1][coada[1][k]];
						if(myabs(pixelSus.r, pixelStart.r)+myabs(pixelSus.g, pixelStart.g)+myabs(pixelSus.b, pixelStart.b) <= threshold)
						{
	//						printf("Am intrat in 1\n");
							//adaug in coada
							coada[0][prim] = coada[0][k] - 1;
							coada[1][prim] = coada[1][k];
							prim++;
							//marchez locu ca fiind ocupat
							zoneMatrix[coada[0][k] - 1][coada[1][k]] = zona;
							image[coada[0][k] - 1][coada[1][k]] = pixelStart;
	//						printf("Am iesit din 1\n");
						}
					}
					//verific pixel jos
					if(coada[0][k]<infoHeader.height - 1 && zoneMatrix[coada[0][k] + 1][coada[1][k]]==0)
					{
						pixelJos = image[coada[0][k]+1][coada[1][k]];
						if(myabs(pixelJos.r, pixelStart.r)+myabs(pixelJos.g, pixelStart.g)+myabs(pixelJos.b, pixelStart.b) <= threshold)
						{
		//					printf("Am intrat in 2\n");
							coada[0][prim] = coada[0][k] + 1;
							coada[1][prim] = coada[1][k];
							prim++;
							zoneMatrix[coada[0][k] + 1][coada[1][k]] = zona;
							image[coada[0][k] + 1][coada[1][k]] = pixelStart;
		//					printf("Am iesit din 2\n");
						}
					}
					//verific pixel stanga
					if(coada[1][k]>0 && zoneMatrix[coada[0][k]][coada[1][k] - 1]==0)
					{
						pixelStanga = image[coada[0][k]][coada[1][k]-1];
						if(myabs(pixelStanga.r, pixelStart.r)+myabs(pixelStanga.g, pixelStart.g)+myabs(pixelStanga.b, pixelStart.b) <= threshold)
						{
		//					printf("Am intrat in 3\n");
							coada[0][prim] = coada[0][k];
							coada[1][prim] = coada[1][k] - 1;
							prim++;
							zoneMatrix[coada[0][k]][coada[1][k] - 1] = zona;
							image[coada[0][k]][coada[1][k] - 1] = pixelStart;
		//					printf("Am iesit din 3\n");
						}
					}
					//verific pixel dreapta
					if(coada[1][k]<infoHeader.width - 1 && zoneMatrix[coada[0][k]][coada[1][k] + 1]==0)
					{
						pixelDreapta = image[coada[0][k]][coada[1][k]+1];
						if(myabs(pixelDreapta.r, pixelStart.r)+myabs(pixelDreapta.g, pixelStart.g)+myabs(pixelDreapta.b, pixelStart.b) <= threshold)
						{
		//					printf("Am intrat in 4\n");
							coada[0][prim] = coada[0][k];
							coada[1][prim] = coada[1][k] + 1;
							prim++;
							zoneMatrix[coada[0][k]][coada[1][k] + 1] = zona;
							image[coada[0][k]][coada[1][k] + 1] = pixelStart;
		//					printf("Am iesit din 4\n");
						}
					}
				}
				//resetez coada si ma pregatesc pt urmatorul lee
			}

		}

		free(coada);
		free(coada[0]);
		free(coada[1]);


		short ii,jj;
		for(i=0; i<infoHeader.height;i++)
		{
			for(j=0; j<infoHeader.width; j++)
			{

				if(checkPixelBorder(i, j, infoHeader.height, infoHeader.width, zoneMatrix))
				{
					ii = (short)i + 1;
					jj = (short)j + 1;
					fwrite(&ii, sizeof(short), 1, fileWrite);
					fwrite(&jj, sizeof(short), 1, fileWrite);
					fwrite(&image[i][j].r, sizeof(unsigned char), 1, fileWrite);
					fwrite(&image[i][j].g, sizeof(unsigned char), 1, fileWrite);
					fwrite(&image[i][j].b, sizeof(unsigned char), 1, fileWrite);
				}
			}
		}
}


void decompressImage(char *FileName, bmpInfoHeader infoHeader, bmpFileHeader fileHeader)
{
	FILE *file;
	file = fopen(FileName, "rb");
	int i, j;
	pixel **decompressed;
	decompressed = calloc(infoHeader.height+1, sizeof(pixel*));
	for(i=0; i < infoHeader.height; i++)
		decompressed[i] = calloc(infoHeader.width+1, sizeof(pixel));


	fseek(file, fileHeader.imageDataOffset, SEEK_SET);

	long fileLen;
	fseek(file, 0, SEEK_END);
	fileLen=ftell(file) - fileHeader.imageDataOffset;
	fseek(file, fileHeader.imageDataOffset, SEEK_SET);

	printf("%ld ", fileLen);

	archive ar;
	pos *blackPixels;
	int start=0;
	blackPixels = calloc(infoHeader.biSizeImage, sizeof(pos));
	while(!feof(file))
	{
		fread(&ar, sizeof(archive), 1, file);
		//printf("%hi %hi %hhu %hhu %hhu\n", ar.x, ar.y, ar.p.r, ar.p.g, ar.p.b);
		decompressed[ar.x-1][ar.y-1] = ar.p;
		if(ar.p.r == 0 && ar.p.g == 0 && ar.p.b == 0)
		{
			blackPixels[start].x = ar.x - 1;
			blackPixels[start].y = ar.y - 1;
			start++;
		}
	}
	fclose(file);
	//printf("start=: %d\n", start);
	pixel pixelCur;
	int k;
	int ok = 1;
	for(i=1; i<infoHeader.height-1;i++)
		for(j=1; j<infoHeader.width-1; j++)
		{
			//ok = 1;
			for(k=0; k < start && ok==1; k++)
				if(blackPixels[k].x == i && blackPixels[k].y == j)
					ok=0;
			if(ok==1 && decompressed[i][j].r == 0 && decompressed[i][j].g == 0 && decompressed[i][j].b == 0)
			{
				pixelCur = decompressed[i][j-1];
				while(decompressed[i][j].r == 0 && decompressed[i][j].g == 0 && decompressed[i][j].b == 0)
				{
					decompressed[i][j] = pixelCur;
					j++;
				}
			}
			else if(ok==0)
			{
				printf("Start: %d %d\n", blackPixels[k].x, blackPixels[k].y);
				while(decompressed[i][j].r == 0 && decompressed[i][j].g == 0 && decompressed[i][j].b == 0)
					{
						j++;
						printf("j=%d\n", j);
					}
				//j--;
				ok = 1;
			}


		}


	flipImage(&decompressed, infoHeader);
	writeImage("decompressed.bmp", infoHeader, fileHeader, decompressed);
}

int main(void)
{
	pixel **Image = NULL;
	bmpInfoHeader infoHeader;
	bmpFileHeader fileHeader;

	FILE *filePtr;
	filePtr = fopen("input.txt", "r");
	char imageName[40];
	int threshold;
	char archiveName[40];

	while(1)
	{
		fscanf(filePtr, "%s", imageName);
		fscanf(filePtr, "%d", &threshold);
		fscanf(filePtr, "%s", archiveName);
		if(feof(filePtr))
			break;

		Image = LoadImage(imageName, &infoHeader, &fileHeader);
		RGBtoBW(imageName, infoHeader, fileHeader);
		applyFilters(imageName, infoHeader, fileHeader);
		compressImage(Image, infoHeader, fileHeader, threshold);
		decompressImage(archiveName, infoHeader, fileHeader);
	}

	fclose(filePtr);

	return 0;
}
