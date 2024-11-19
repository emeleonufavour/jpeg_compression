# JPEG compression.

This repository contains an algorithm I am working on and using to learn JPEG image compression.
The compression pipeline so far goes this way:

1.  I first of all convert the image from the RGB color space to the YCrCb color based on the idea that
    the human eye is more sensitive to brightness. This is known as Color space conversion, a subsampling factor of 2 is applied to each 4 x 4 pixel block of the image data.

2.  From the pixel value data, we will need to generate the frequency data which we will achieve by using a
    Discrete Cosine Transform (DCT) which transforms the pixel data into the frequency domain. We are using the frequency data because the human eye tends to notice low frequency data as opposed to high frequency data.

3.  After applying DCT, we perform quantization on the data. Removing the values that tend to zero signaling
    high frequency data and focus on the large values which resulted from low frequency value. We remove the zeros and keep the non zeros

4.  To keep things shorter, I perform a Run Length Encoding for the quantisized data.

5.  The final compression process involves applying Huffman coding to the final image data.
