#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <chrono>
#include <string>

using std::cin;
using std::cout;
using std::endl;


#define Escape	27
#define Enter	13


enum CarState
{
	Idle,
	Acceleration,
	Slowdown,
	Coasting
};






HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);


void ClearMessageFill(short x, short y)
{

	SetConsoleCursorPosition(hConsole, {x, y });
	cout << std::string(80, ' ');
	SetConsoleCursorPosition(hConsole, { x, y });
}


void PrintAt(short x, short y, const std::string& text)
{
	constexpr int LINE_WIDTH = 30;

	SetConsoleCursorPosition(hConsole, { x, y });

	cout << text;

	// Затираем хвост от предыдущего, более длинного значения.
	if (text.length() < LINE_WIDTH)
	{
		cout << std::string(LINE_WIDTH - text.length(), ' ');
	}
}


void PrintStepMessage(const char* message)
{
	ClearMessageFill(0, 8);
	cout << message << endl;
}


#define MIN_TANK_CAPACITY	 20
#define MAX_TANK_CAPACITY	120

class Tank
{
	const int CAPACITY;
	double fuel_level;


public:


	Tank(int capacity) :CAPACITY
	(
		capacity < MIN_TANK_CAPACITY ? MIN_TANK_CAPACITY :
		capacity > MAX_TANK_CAPACITY ? MAX_TANK_CAPACITY :
		capacity
	)
	{
		this->fuel_level = 0;
		//cout << "Tank is ready " << this << endl;
	}
	~Tank()
	{
		//cout << "Tank is over " << this << endl;
	}
	double get_fuel_level()const
	{
		return fuel_level;
	}


	int GetCapacity() const { return CAPACITY; }
	double GetFuelLevel()	{ return fuel_level; }


	//заправка
	bool TryFill(double amount)
	{
		if (amount < 0)
		{
			PrintStepMessage("Заправщик пытаеться слить Ваш бензин! amount < 0");
			return false;
		}

		fuel_level += amount;
		if (fuel_level > CAPACITY)fuel_level = CAPACITY;

		PrintStepMessage("Машина заправлена");
		return true;
	}

	//попытка получить топливо 

	bool TryGiveFuel(double amount)
	{
		if (amount < 0)return false;
		if (amount > fuel_level) return false;

		fuel_level -= amount;
		return true;
	}

	bool HasFuel() { return fuel_level > 0; }

};


#define MIN_ENGINE_CONSUMPTION	 4
#define MAX_ENGINE_CONSUMPTION	30
class Engine
{
	const double CONSUMPTION;		// расход на 100 км
	double consumption_per_second;	//расход за 1 сек
	bool is_started;
public:
	Engine(double consumption) :CONSUMPTION
	(
		consumption < MIN_ENGINE_CONSUMPTION ? MIN_ENGINE_CONSUMPTION :
		consumption > MAX_ENGINE_CONSUMPTION ? MAX_ENGINE_CONSUMPTION :
		consumption
	)
	{
		consumption_per_second = CONSUMPTION * 3e-5;	//3 * 10^(-5)
		is_started = false;
		//cout << "Engine is ready:\t" << this << endl;
	}
	~Engine()
	{
		//cout << "Engine is over:\t" << this << endl;
	}


	double	get_consumption_per_second() { return consumption_per_second; }
	
																	
	bool	IsStarted()const				{ return is_started; }


	void start()
	{
		is_started = true;
		PrintStepMessage("Двигатель запущен.");
	}
	void stop()
	{
		is_started = false;
		PrintStepMessage("Двигатель заглушён.");
	}
};




//состояние машины 
struct CarRuntimeState
{
	double speed = 0.0;

	bool acceleration_requested = false;
	bool braking_requested = false;

	CarState GetCarState() const
	{
		if (acceleration_requested)
			return CarState::Acceleration;

		if (braking_requested)
			return CarState::Slowdown;

		if (speed > 0.0)
			return CarState::Coasting;

		return CarState::Idle;
	}



	void ClearFlags()
	{
		acceleration_requested = false;
		braking_requested = false;
	}




};




class Car
{
	Engine engine;
	Tank tank;
	
	CarRuntimeState state;


public:

	Car(double consumtion, int capacity = 50) :engine(consumtion), tank(capacity)
	{

		//cout << "Your car is ready to go, press Enter to get in\t" << this << endl;
	}
	~Car()
	{
		//cout << "Car is over:\t\t\t\t\t" << this << endl;
	}

	//попытка завести двигатель
	bool TryStartEngine()
	{
		//проверка наличия топлива в баке
		if (tank.HasFuel()) { engine.start(); return true; }
		else
		{
			PrintStepMessage("В баке отсутствует топливо, невозможно завести машину");
			return false;
		}
		
	}

	//попытка заглушить двигатель
	bool TryStopEngine()
	{
		if (state.GetCarState() == CarState::Idle) { engine.stop(); return true; }
		else
		{
			PrintStepMessage("Нельзя глушить машину во время движения.");
			return false;
		}
	}

	//попытка ускориться 
	bool TryAccelerate()
	{
		if (!engine.IsStarted())
		{
			PrintStepMessage("Нельзя разгоняться: двигатель заглушён.");
			return false;
		}

		state.acceleration_requested = true;
		state.braking_requested = false;
		return true;
	}


	//попытка остановиться
	bool TryBrake()
	{
		if (state.speed <= 0.0)
		{
			PrintStepMessage("Машина уже стоит.");
			return false;
		}

		state.braking_requested = true;
		state.acceleration_requested = false;
		return true;
	}

	//попытка заправить машину 
	bool TryFill(double amount)
	{
		if (engine.IsStarted())
		{
			PrintStepMessage("Нельзя заправлять машину при работающем двигателе.");
			return false;
		}

		return tank.TryFill(amount);


	}





	void ScreenPanel()
	{
		PrintAt(0, 0, "Запас топлива: " + std::to_string(tank.GetFuelLevel()) + " л.");
		PrintAt(0, 1, "Скорость: " + std::to_string(state.speed) + " км/ч");
		
	}




	void Update(double delta_time)
	{
		//обновляем состояние только при включенном двигателе
		if (!engine.IsStarted()) return;


		switch (state.GetCarState())
		{
		case CarState::Acceleration:

			{
				state.speed += 20.0 * delta_time;
				if (state.speed > 250) state.speed = 250;

				int k = 0;
				if (state.speed >= 0 && state.speed < 60) k = 66;
				else if (state.speed >= 60 && state.speed < 100) k = 46;
				else if (state.speed >= 100 && state.speed < 140) k = 66;
				else if (state.speed >= 141 && state.speed < 200) k = 83;
				else if (state.speed >= 200 && state.speed <= 250) k = 100;

				tank.TryGiveFuel(engine.get_consumption_per_second() * k * delta_time);
			}
			
			break;

		case CarState::Slowdown:
			state.speed -= 40.0 * delta_time;

			if (state.speed < 0.0) state.speed = 0.0;
			tank.TryGiveFuel(engine.get_consumption_per_second() * delta_time);

			break;

		case CarState::Coasting:
			// Машина постепенно теряет скорость сама.
			state.speed -= 1.0 * delta_time;

			if (state.speed < 0.0) 	state.speed = 0.0;
			tank.TryGiveFuel( engine.get_consumption_per_second() * delta_time);

			break;

		case CarState::Idle:
			// Двигатель работает, даже если машина стоит.
			tank.TryGiveFuel(
				engine.get_consumption_per_second() * delta_time
			);
			break;
		}

		if (!tank.HasFuel())
		{
			engine.stop();
			state.speed = 0.0;
		}

		state.ClearFlags();

	}



};


class Driver
{
	Car* CurrentCar = nullptr; 


public:

	//в машине ли водитель
	bool IsInCar() const
	{
		return CurrentCar != nullptr;
	}


	//вход выход из машины
	void SitInCar(Car* car) 
	{ 
		CurrentCar = car; 
		PrintStepMessage("Водитель сел в машину.");
	}
	void OutOfCar() 
	{ 
		CurrentCar = nullptr; 
		PrintStepMessage("Водитель вышел из машины.");
	}


	//попытка завести машину 
	bool TryStartCar()
	{
		if (CurrentCar == nullptr)
		{
			PrintStepMessage("Водитель не в машине, невозможно завести двигатель!");
			return false;
		}

		return CurrentCar->TryStartEngine();
	}

	//попытка остановить машину 
	bool TryStopCar()
	{
		if (CurrentCar == nullptr)
		{
			PrintStepMessage("Водитель не в машине, невозможно заглушить двигатель!");
			return false;
		}

		return CurrentCar->TryStopEngine();
	}


	//попытка ускорить машину
	bool TryAccelerate()
	{
		if (CurrentCar == nullptr)
		{
			PrintStepMessage("Водитель не в машине, невозможно нажать газ.");
			return false;
		}

		return CurrentCar->TryAccelerate();
	}

	//попытка замедлить машину
	bool TryBrake()
	{
		if (CurrentCar == nullptr)
		{
			PrintStepMessage("Водитель не в машине, невозможно тормозить.");
			return false;
		}

		return CurrentCar->TryBrake();
	}


	//попытка заправить машину 
	bool TryFill(Car* car)
	{
		if (CurrentCar != nullptr)
		{
			PrintStepMessage("Что бы заправиться нужно выйти из машины.");
			return false;
		}

		ClearMessageFill(0, 8);
		cout << "Введите количество топлива: ";

		double amount;
		cin >> amount;

		return 	car->TryFill(amount);


	}

};









void main()
{
	setlocale(LC_ALL, "Russian");


	Car bmw(10, 70);
	Driver driver;


	ClearMessageFill(0, 12);
	cout << "Enter - сесть/выйти \nI - запуск двигатель\nO - заглушить двигатель\n"
		"W - газ\nS - тормоз\nF - заправка\nEsc - выход";
	ClearMessageFill(0, 0);


	bool program_running = true;

	std::chrono::steady_clock::time_point previous_time = std::chrono::steady_clock::now();

	while (program_running)
	{
		std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();

		std::chrono::duration<double> elapsed = current_time - previous_time;
		previous_time = current_time;

		if (_kbhit())
		{
			char key = _getch();

			switch (key)
			{
			case Enter:
				if (driver.IsInCar())
					driver.OutOfCar();
				else
					driver.SitInCar(&bmw);
				break;

			case 'i':
			case 'I':
				driver.TryStartCar();
				break;

			case 'o':
			case 'O':
				driver.TryStopCar();
				break;

			case 'w':
			case 'W':
				driver.TryAccelerate();
				break;

			case 's':
			case 'S':
				driver.TryBrake();
				break;


			case 'f':
			case 'F':
				driver.TryFill(&bmw);
				break;


			case Escape:
				program_running = false;
				break;
			}
		}

		bmw.Update(elapsed.count());
		bmw.ScreenPanel();

		Sleep(50);
	}


}