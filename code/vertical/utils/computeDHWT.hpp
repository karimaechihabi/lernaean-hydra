#ifndef COMPUTE_DHWT_HPP
#define COMPUTE_DHWT_HPP


int GetSymbol(double);
void GenerateLevelFiles(std::string, std::string, int, int);

int computeDHWT(char* infile, char* inputDir, int count, int size, char* outputDir);


#endif
