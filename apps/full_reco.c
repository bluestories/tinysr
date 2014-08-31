// This app runs utterance detection on audio streamed in via stdin.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "tinysr.h"

#define READ_SAMPS 128

int keep_reading;
void sig_handler(int signo) {
	printf("SIGINT caught, stopping.\n");
	keep_reading = 0;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage:\n");
		printf("<command to produce audio> | full_reco <speech_model>\n");
		printf("Expects the input to be 16000 Hz mono 16-bit signed little endian raw audio.\n");
		printf("Expects a file called speech_model in the same directory.\n");
		return 1;
	}

	// Allocate a context.
	fprintf(stderr, "Allocating context.\n");
	tinysr_ctx_t* ctx = tinysr_allocate_context();
	ctx->input_sample_rate = 16000;
	printf("Loaded up %i words.\n", tinysr_load_model(ctx, argv[1]));
	samp_t array[READ_SAMPS];
	keep_reading = 1;
	signal(SIGINT, sig_handler);
	while (keep_reading) {
		// Try to read in samples.
		size_t samples_read = fread(array, sizeof(samp_t), READ_SAMPS, stdin);
		if (samples_read == 0) break;
		// Feed in the samples to our recognizer.
		tinysr_feed_input(ctx, array, (int)samples_read);
		tinysr_detect_utterances(ctx);
		tinysr_recognize_utterances(ctx);
	}
	fprintf(stderr, "Freeing context. Processed %i samples.\n", ctx->processed_samples);
	tinysr_free_context(ctx);

	return 0;
}
