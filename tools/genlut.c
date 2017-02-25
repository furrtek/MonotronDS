// LookUp Table generator for MonotronDS
// 2017 furrtek

// Measurements:
// VCO=0 lowest key:	23.15Hz
// VCO=0 highest key:	56.29Hz
// VCO=0.5 center key:	542.44Hz
// VCO=1 lowest key:	2261.54Hz
// VCO=1 highest key:	5478.26Hz
// Range: 8 octaves

// f = a^(n+pitch)
// a = 2^(1/12) = 1.0594630943592952645618252949463
// fmin = a^(54+0) = 22.63Hz
// fmax = a^(54+15+81) = 5792.6Hz
// Lowest key: n = 0
// Highest key: n = 15

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define ALPHA 1.0594630943592952645618252949463
#define SAMPLE_RATE 22050

int main(int argc, char *argv[]) {
	FILE * file;
	unsigned int step;
	unsigned long int value;
	unsigned long int * buffer;		// unsigned long int MUST be 32bit
	double frequency;
	
	buffer = malloc(32768 * 4);		// 128kB
	if (buffer == NULL) return 1;
	
	// Generate VCO table
	puts("Generating VCO table...");
	file = fopen("../data/vco.bin", "wb");
	if (file == NULL) return 1;
	// The ribbon is exactly 234px wide and represents 16 keys
	// The gap between keys is 234/(16-1) = 15.6px =~ 16px
	// 8 octaves * 12 keys * 16 steps = 1536
	for (step = 0; step < 1536; step++) {
		frequency = pow(ALPHA, 54.0 + (double)step / 16.0);
		value = (unsigned long int)(frequency / SAMPLE_RATE * 0xFFFFFFFFU);
		buffer[step] = value;
	}
	fwrite(buffer, 4, 1536, file);
	fclose(file);
	
	// Generate LFO table
	puts("Generating LFO table...");
	file = fopen("../data/lfo.bin", "wb");
	if (file == NULL) return 1;
	for (step = 0; step < 1024; step++) {
		frequency = pow(2, (double)step / 160.0) - 0.93;
		value = (unsigned long int)(frequency / SAMPLE_RATE * 0xFFFFFFFFU);
		buffer[step] = value;
	}
	fwrite(buffer, 4, 1024, file);
	fclose(file);
	
	system("PAUSE");
	
	return 0;
}
