import PikaStdLib
import machine

time = machine.Time()
pwm = machine.PWM()
pwm.setPin('PA8')
pwm.setFrequency(2000)
pwm.setDuty(0.5)
pwm.enable()
mem = PikaStdLib.MemChecker()

while True:
    mem.now()
    time.sleep_ms(500)
    pwm.setDuty(0.5)
    time.sleep_ms(500)
    pwm.setDuty(0.001)
    
