#include <iostream>
#include<conio.h>

using std::cin;
using std::cout;
using std::endl;

#define ESCAPE 27
#define ENTER 13

#define MIN_TANK_CAPACITY 20
#define MAX_TANK_CAPACITY 120


class Tank
{
	const int CAPACITY;
	double fuel_level;

public: 
	
	Tank(int capacity) : 
	CAPACITY
	(
		capacity < MIN_TANK_CAPACITY ? MIN_TANK_CAPACITY : 
		capacity > MAX_TANK_CAPACITY ? MAX_TANK_CAPACITY : 
		capacity
	)
	{
		this->fuel_level = 0;

		cout << "Tank is ready " << this << endl;
	}

	~Tank()
	{
		cout << "Tank is over " << this << endl;
	}



	//приватные методы ................................................................................................................................
	// 
	// 

	double get_fuel_level()const
	{
		return fuel_level;
	}





	//заправка бака
	void Fill(int amount)
	{	
		if (amount < 0) return;
		fuel_level += amount;
		if (fuel_level > CAPACITY) fuel_level = CAPACITY;
	}

	//расход топлива
	double give_fuel(double amount) // лучше перевести метод на TRY 
	{
		if (amount < 0) return fuel_level;
		fuel_level -= amount;
		if (fuel_level < 0) fuel_level = 0;

		return fuel_level;
	}

	void info()const
	{
		cout << "Capacity: \t " << CAPACITY << " liters. \n";
		cout << "Fuel level: \t " << fuel_level << " liters. \n";
	}


};




#define MIN_ENGIN_CONSUMPTION 4
#define MAX_ENGIN_CONSUMPTION 30


//движок 
class Engine
{
	const double CONSUMPTION; //расход на 100 км
	double consumption_per_second; //расход  за 1 секунду


public:
	Engine(double consumption) : CONSUMPTION
	(
		consumption < MIN_ENGIN_CONSUMPTION ? MIN_ENGIN_CONSUMPTION :
		consumption > MAX_ENGIN_CONSUMPTION ? MAX_ENGIN_CONSUMPTION :
		consumption
	)

	{
		consumption_per_second = CONSUMPTION * 3e-5;
		cout << "Engin is ready: " << this << endl;
	}

	~Engine()
	{
		cout << "Engin is over: " << this << endl;
	}

	void info()const
	{
		cout << "Consumption:\t\t" << CONSUMPTION << " liters/km \n";
		cout << "Consumption per sec.:\t" << consumption_per_second << " liters/sec. \n";
	}

};

class Car
{
	Engine engine;
	Tank tank;

	bool driver_inside; //как костыль - зачем машине знать о наличии водител€? 


public:
	Car(double consumption, int capacity = 50) : engine(consumption), tank(capacity)
	{
		driver_inside = false;
		cout << "Your car is ready to go, press Enter to get in " << this << endl;
	}

	~Car()
	{
		cout << "Car is over: " << this << endl;
	}

	 
	void get_in() //костыль - машина сажает водител€ внутрь
	{
		driver_inside = true;
		panel();
	}

	void get_out() //костыль - машина высаживает водител€.
	{
		driver_inside = false;
	}


	void control()
	{
		char key = 0;

		do
		{
			key = _getch(); //ловим нажатую клавишу

			switch (key)
			{
			case ENTER: 
				if (driver_inside) get_out();
				else get_in();
				break;
			}



		} while (key != ESCAPE);
	}


	void panel()
	{
		while (driver_inside)
		{
			system("CLS");
			cout << "Fuel level: " << tank.get_fuel_level() << endl;
		}
	}


};




//#define TANK_CHECK
//#define ENGINE_CHECK
#define CAR_CHECK


void main()
{
	setlocale(LC_ALL, "Russian");


#ifdef TANK_CHECK
	Tank tank(40);

	int amount;

	while (true)
	{
		cout << "¬ведите объем топлива: "; cin >> amount;

		tank.Fill(amount);
		tank.info();
	}

#endif //TANK_CHECK;



#ifdef ENGINE_CHECK
	Engine engine(10);
	engine.info();
#endif // ENGINE_CHECK

#ifdef CAR_CHECK


	Car bmw(10, 70);
	bmw.control();


#endif // CAR_CHECK

}



