# Changelog

## v0.1.0

- Initial release: dying battery fuzz inspired by fuzz face bias pot
- Drive knob (input gain, audio taper 1× – 10×)
- Bias knob (transistor operating point, skewed taper — breaks up around 9 o'clock)
- Output trim (−20 to +6 dB)
- Asymmetric dead-zone clipper: hard snap on positive side, softer on negative
- DC block HPF (~30 Hz) removes bias-introduced offset
