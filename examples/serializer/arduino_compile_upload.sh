sketch="./arduino_producer/arduino_producer.ino"
arduino-cli compile -b arduino:avr:uno $sketch && arduino-cli upload -p /dev/ttyACM0 -b arduino:avr:uno $sketch -t -v
