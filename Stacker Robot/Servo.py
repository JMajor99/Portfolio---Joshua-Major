import RPi.GPIO as GPIO
import time

# Pin setup
servo_pin = 18  # PWM pin (GPIO18)
GPIO.setmode(GPIO.BCM)
GPIO.setup(servo_pin, GPIO.OUT)

# Start PWM at 50Hz
pwm = GPIO.PWM(servo_pin, 50)  # 50Hz = standard for servos
pwm.start(0)

def set_angle(angle):
    """Move servo to given angle (0–180)."""
    duty = 2 + (angle / 18)  # Maps angle to duty cycle
    pwm.ChangeDutyCycle(duty)
    time.sleep(0.5)
    pwm.ChangeDutyCycle(0)  # Prevents jitter

try:
    while True:
        user_input = input("Enter angle (0–180): ")

        if not user_input.isdigit():
            print("Please enter a number between 0 and 180.")
            continue

        angle = int(user_input)
        if 0 <= angle <= 180:
            set_angle(angle)
        else:
            print("Invalid angle. Must be between 0 and 180.")

except KeyboardInterrupt:
    print("\nStopped by user")

finally:
    pwm.stop()
    GPIO.cleanup()
