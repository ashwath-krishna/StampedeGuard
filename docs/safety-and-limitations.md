# Safety, scope, and limitations

## Intended scope
This is an educational prototype for experimenting with single-lane passage-event detection. It is not a certified crowd-control product or a safety instrument.

## Known limitations in the current firmware
- **One sensing point:** the VL53L1X observes a narrow field at one location, not an entire venue.
- **Entry events only:** the firmware increments a counter when it detects an object/person-like distance change. It does not detect direction and does not subtract exits.
- **Simultaneous passages:** multiple people crossing together may be counted as one event or missed.
- **Object ambiguity:** the sensor measures distance, not identity; other objects can trigger the same threshold.
- **Calibration dependence:** performance depends on mounting position, lane geometry, sensor field of view, ambient conditions, and the empty-lane baseline.
- **Not actual occupancy:** the displayed count is cumulative detected entries since reset, not the number of people currently inside.
- **Not actual density:** the firmware divides the cumulative entry count by a configured area. That is not a validated spatial density measurement.
- **Status labels are not safety assessments:** SAFE/CAUTION/DANGER are generated from the simple calculated indicator and user-configured thresholds. They do not establish that a venue is safe.
- **No certified capacity calculation:** the current code does not calculate remaining capacity.

## Concept image
The rally illustration is a visual communication mockup. Its 6,054 people and 743 capacity-remaining figures are illustrative dashboard content only; they are not sensor readings, field results, or a verified estimate of the pictured crowd.

## Safe development practice
- Test only in a controlled, low-risk environment with a clear sensor lane.
- Do not obstruct exits or alter real crowd movement to test the prototype.
- Do not use the prototype to decide whether a real venue should admit more people or whether an evacuation is needed.
- Do not represent concept-image figures as measured results.
- Validate any future counting or density algorithm against independent ground truth before making performance claims.
- A real deployment would require professional venue-safety review, redundant sensing, fault handling, validated thresholds, and compliance with applicable regulations.

## Public repository security
The firmware template replaces the original demo access-point password with a placeholder. Set a private password locally and never commit real credentials, tokens, or personal data.
