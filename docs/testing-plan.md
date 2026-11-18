# Testing and evaluation plan

This plan describes tests to run; it does not claim that these experiments have already been completed.

## Questions
1. How reliably does the sensor detect one person passing through the monitored lane?
2. How often does it miss an event or count one passage more than once?
3. How does performance change with walking speed, sensor placement, and object size?
4. What happens when two people pass close together?

## Suggested controlled test matrix

| Test | Setup | Record |
|---|---|---|
| Empty lane | No one crosses for a fixed interval | False-trigger count |
| Single passage | One person crosses repeatedly | Detected events vs reference |
| Different speeds | Slow, normal, and brisk walking | Misses and duplicate detections |
| Lane clearance | Person crosses, then lane clears | Rearming time |
| Close-following passages | Two people cross with a short gap | Count accuracy |
| Side movement | Person moves near the edge of the sensor view | False triggers / misses |
| Sensor obstruction | Sensor view is unintentionally blocked | Health indication and recovery |

## Metrics
- **Count accuracy:** detected passage events compared with a manually recorded reference.
- **Miss rate:** missed reference passages divided by total reference passages.
- **Duplicate rate:** extra detections divided by total reference passages.
- **False-trigger rate:** detections during empty-lane observation time.
- **Rearming time:** time from a detected passage until the detector is ready for the next one.

Report the number of trials, conditions, and failures. Do not publish performance percentages until the tests have actually been run and the data has been retained.

## Data table template

| Trial | Condition | Reference passages | Detected events | Misses | Duplicates | Notes |
|---|---|---:|---:|---:|---:|---|
| 1 | Single person, normal pace | | | | | |
| 2 | Single person, slow pace | | | | | |
| 3 | Two close-following people | | | | | |
| 4 | Empty lane | 0 | | — | | |
