import machine

import PikaStdLib

pin = machine.GPIO()
rgb = machine.RGB()
mem = PikaStdLib.MemChecker()

pin.setPin('PA8')
pin.setMode('out')
pin.enable()

rgb.init()
rgb.enable()

print('task demo')
print('mem used max:')
mem.max()


def rgb_task():
    rgb.flow()
    mem.now()


def led_task():
    if pin.read():
        pin.low()
    else:
        pin.high()


task = machine.Task()

task.call_period_ms(rgb_task, 50)
task.call_period_ms(led_task, 500)

task.run_forever()

