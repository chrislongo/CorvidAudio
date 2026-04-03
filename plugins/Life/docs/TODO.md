# Life - TODO

- [x] Change IT/OT from on/off toggles to drive knobs (0-100). 0 = bypassed/clean, higher values push harder into transformer saturation. HPF stays always-on when drive > 0. Gives 5 knobs in a clean horizontal row.
- [x] Wide toggle: Change to pill with Wide text inside. Filled engaged, hallow not enaged. Move to top right.
- [x] Change THD to "Harmonics" or "Harm" if not enough room
- [x] Change Sat to "Tube"
- [x] There are still clicks and pops when Wide is engaged
- [x] Change transformer controls to only one knob controlling both simultaneously (does not make sense to engage one or the other)
- [x] Don't use the Level-Loc transformer algo. Use something closer to what's found in a vintage SSL console (E-Channel, G-Channel)
- [x] Order: Noise -> THD -> Tube -> Transformers
- [x] Default: All knobs at zero
- [ ] The way Wide works is just wrong. Think of it this way, each component has a touch of varience to its algorithm. In a classic console, two channels are meant to sound exactly the same, but they are slightly off due to varience in analog parts. The way it's modeled now it's hopping all over. It should add a touch of varieince between L and R channels that makes the sound a bit wider. It does not even have to be random, you can model two slightly different variants of the same algo per channel.