import numpy as np

doremi = [
	16, 17.1, 18.364, 19, 21.2, 21.827, 23, 25, 26, 28.2, 29.2, 30.868,
	32.603, 35, 36.708, 39, 41.303, 44.2, 46, 49.1, 52, 55, 58, 62,
	65, 69, 73.416, 78, 82, 87, 93, 97.899, 103.828, 110.1, 117, 123.472,
	131, 139, 147, 156, 165, 175, 185, 196, 207.656, 220, 233, 247,
	262, 277, 293.66, 311, 330, 349, 370, 392, 415.1, 440, 466, 494,
	523, 554, 587, 622, 659, 698, 740.1, 784, 831, 880, 932, 988,
	1047, 1109, 1175, 1244, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,
	2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,
	4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902,
	8372, 8870, 9397, 9956, 10548, 11175, 11840, 12544
]

# 約数を高速に求める関数（改造版）
def make_divisors(n):
	divisors = []
	for i in range(1, int(n**0.5)+1):
		if n % i == 0 and n//i < 65535:
			if i != n // i:
				divisors.append((i, n//i))
				divisors.append((n//i, i))
			else:
				divisors.append((i, n//i))
	return divisors


def calcTimParameter(tim_clock, interrupt_hz):
	# clock / ab = hz
	ab = int(round(tim_clock / interrupt_hz, 0))

	params = make_divisors(ab)
	if params == []:
		# 条件に一致する組が存在しない場合は
		# 誤差の大きい方に切り替えて再計算（逆四捨五入）
		f = int(((tim_clock / interrupt_hz) * 10)) % 10
		if f < 5:
			ab += 1
		else:
			ab -= 1
		params = make_divisors(ab)
	
	params = sorted(params)
	real_freq = tim_clock/ab
	return (real_freq, params)


def main():
	clock = 72000000

	EF = False
	EC = 0
	vals = []
	for row in doremi:
		real_freq, values = calcTimParameter(clock, row)
		print(row, ",", '{:.2f}'.format(real_freq), ",", end="")
		if values == []:
			print("")
		ff = False
		for psc, arr in values:
			if psc > 100:
				continue
			ff = True
			print('%5d , %5d' % (psc-1, arr-1))
			vals.append((psc-1, arr-1))
			break
		if ff == False:
			EF = True
			EC += 1
			print("ERROR!")
	print("\n")
	if EF:
		print("!!! ERROR !!! -> " + str(EC))
	else:
		print("OK")
	print("\n")
	print("uint16_t psc[128] = {")
	for row in range(12):
		print("0, ", end="")
	for psc, _ in vals:
		print(psc, ", ", end="")
	print("};")

	print("uint16_t arr[128] = {")
	for row in range(12):
		print("0, ", end="")
	for _, arr in vals:
		print(arr, ", ", end="")
	print("};")

if __name__ == "__main__":
	main()
	











