#include "Arduino.h"

#include "Feature.h"
#include "ofxXmlSettings.h"
#include <direct.h> // for getcwd
#define M_PI_2     1.57079632679489661923

namespace mapinect {

#define		ARDUINO_CONFIG		"ArduinoConfig:"

#define		ID_MOTOR_1			1
#define		ID_MOTOR_2			2
#define		ID_MOTOR_4			4
#define		ID_MOTOR_8			8

#define		ANGLE_UNDEFINED		MAXCHAR
#define		ANGLE_DEFAULT		0
#define		KEY_UNDEFINED		""

#define		MOTOR_ANGLE_OFFSET_DEFAULT	128

	static string	COM_PORT;
	static char		KEY_MOVE_1R;
	static char		KEY_MOVE_1L;
	static char		KEY_MOVE_2R;
	static char		KEY_MOVE_2L;
	static char		KEY_MOVE_4R;
	static char		KEY_MOVE_4L;
	static char		KEY_MOVE_8R;
	static char		KEY_MOVE_8L;
	static char		KEY_RESET;
	static char		KEY_PRINT_STATUS;
	static char		RESET_ANGLE1;
	static char		RESET_ANGLE2;
	static char		RESET_ANGLE4;
	static char		RESET_ANGLE8;
	static int		ANGLE_STEP;
	static int		ARM_HEIGHT;
	static int		ARM_LENGTH;
	static int		MOTOR_ANGLE_OFFSET;

	Arduino::Arduino()
	{
		/*angleMotor1 = 8;
		angleMotor2 = 0;
		angleMotor4 = -10;
		angleMotor8 = 18;*/
		angleMotor1 = 0;
		angleMotor2 = 0;
		angleMotor4 = 0;
		angleMotor8 = 0;
	}

	Arduino::~Arduino()
	{
		if (serial.available()){
			serial.close();
		}
	}

	bool Arduino::setup()
	{
		CHECK_ACTIVE false;
		ofxXmlSettings XML;
		char CurrentPath[255];
		getcwd(CurrentPath, 255);
		cout << CurrentPath << endl;
		if(XML.loadFile("Arduino_Config.xml")) {

			COM_PORT = XML.getValue(ARDUINO_CONFIG "COM_PORT", "COM3");

			KEY_MOVE_1R = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_1R", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_1L = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_1L", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_2R = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_2R", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_2L = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_2L", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_4R = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_4R", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_4L = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_4L", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_8R = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_8R", KEY_UNDEFINED).c_str()[0];
			KEY_MOVE_8L = XML.getValue(ARDUINO_CONFIG "KEY_MOVE_8L", KEY_UNDEFINED).c_str()[0];

			KEY_RESET = XML.getValue(ARDUINO_CONFIG "KEY_RESET", KEY_UNDEFINED).c_str()[0];
			KEY_PRINT_STATUS = XML.getValue(ARDUINO_CONFIG "KEY_PRINT_STATUS", KEY_UNDEFINED).c_str()[0];

			ANGLE_STEP = XML.getValue(ARDUINO_CONFIG "ANGLE_STEP", 2);

			ANGLE_STEP = XML.getValue(ARDUINO_CONFIG "HEIGHT", 2);
			ANGLE_STEP = XML.getValue(ARDUINO_CONFIG "LENGTH", 2);

			RESET_ANGLE1 = XML.getValue(ARDUINO_CONFIG "RESET_ANGLE1", ANGLE_DEFAULT);
			RESET_ANGLE2 = XML.getValue(ARDUINO_CONFIG "RESET_ANGLE2", ANGLE_DEFAULT);
			RESET_ANGLE4 = XML.getValue(ARDUINO_CONFIG "RESET_ANGLE4", ANGLE_DEFAULT);
			RESET_ANGLE8 = XML.getValue(ARDUINO_CONFIG "RESET_ANGLE8", ANGLE_DEFAULT);
			MOTOR_ANGLE_OFFSET	= XML.getValue(ARDUINO_CONFIG "MOTOR_ANGLE_OFFSET", MOTOR_ANGLE_OFFSET_DEFAULT);

			ARM_HEIGHT = XML.getValue(ARDUINO_CONFIG "ARM_HEIGHT", 0);
			ARM_LENGTH = XML.getValue(ARDUINO_CONFIG "ARM_LENGTH", 0);

		}

		serial.enumerateDevices();
		if (serial.setup(COM_PORT, 9600)) {
			//reset();
			return true;
		}
		return false;
	}

	void Arduino::exit() {
		CHECK_ACTIVE;
		if (serial.available()){
			serial.close();
		}
	}

	void Arduino::update() {
		CHECK_ACTIVE;
	}

	void Arduino::draw() {
		CHECK_ACTIVE;
	}

	void Arduino::keyPressed (int key) {
		CHECK_ACTIVE;

		if (key == KEY_MOVE_1R) {
			angleMotor1 += ANGLE_STEP;
			sendMotor((char) angleMotor1, ID_MOTOR_1);
		}
		else if (key == KEY_MOVE_1L) {
			angleMotor1 -= ANGLE_STEP;
			sendMotor((char) angleMotor1, ID_MOTOR_1);
		}
		else if (key == KEY_MOVE_2R) {
			angleMotor2 += ANGLE_STEP;
			sendMotor((char) angleMotor2, ID_MOTOR_2);
		}
		else if (key == KEY_MOVE_2L) {
			angleMotor2 -= ANGLE_STEP;
			sendMotor((char) angleMotor2, ID_MOTOR_2);
		}
		else if (key == KEY_MOVE_4R) {
			angleMotor4 += ANGLE_STEP;
			sendMotor((char) angleMotor4, ID_MOTOR_4);
		}
		else if (key == KEY_MOVE_4L) {
			angleMotor4 -= ANGLE_STEP;
			sendMotor((char) angleMotor4, ID_MOTOR_4);
		}
		else if (key == KEY_MOVE_8R) {
			angleMotor8 += ANGLE_STEP;
			sendMotor((char) angleMotor8, ID_MOTOR_8);
		}
		else if (key == KEY_MOVE_8L) {
			angleMotor8 -= ANGLE_STEP;
			sendMotor((char) angleMotor8, ID_MOTOR_8);
		}
		else if (key == KEY_RESET) {
			reset();
		}
		else if (key == KEY_PRINT_STATUS) {
			cout << read() << endl;
			cout << "motor 1: " << angleMotor1 << endl;
			cout << "motor 2: " << angleMotor2 << endl;
			cout << "motor 4: " << angleMotor4 << endl;
			cout << "motor 8: " << angleMotor8 << endl;
		}
	}


	void Arduino::reset()
	{
		CHECK_ACTIVE;
		if (RESET_ANGLE1 != ANGLE_UNDEFINED) {
			angleMotor1 = RESET_ANGLE1;
			sendMotor((char) angleMotor1, ID_MOTOR_1);
		}
		if (RESET_ANGLE2 != ANGLE_UNDEFINED) {
			angleMotor2 = RESET_ANGLE2;
			sendMotor((char) angleMotor2, ID_MOTOR_2);
		}
		if (RESET_ANGLE4 != ANGLE_UNDEFINED) {
			angleMotor4 = RESET_ANGLE4;
			sendMotor((char) angleMotor4, ID_MOTOR_4);
		}
		if (RESET_ANGLE8 != ANGLE_UNDEFINED) {
			angleMotor8 = RESET_ANGLE8;
			sendMotor((char) angleMotor8, ID_MOTOR_8);
		}
	}

	const char *my_byte_to_binary(int x)
	{
		static char b[9];
		b[0] = '\0';

		int z;
		for (z = 256; z > 0; z >>= 1)
		{
			strcat(b, ((x & z) == z) ? "1" : "0");
		}

		return b;
	}

	void Arduino::sendMotor(int value, int id)
	{
		//value += MOTOR_ANGLE_OFFSET;
		if (value < 0){
			value = -value;
			value |= 1 << 7; //MAGIC!
		}
		cout << value <<endl;
		cout << my_byte_to_binary(value) <<endl;
		char id_char = (char) id;
		serial.writeByte(id_char);
		serial.writeByte(value);
	}

	char* Arduino::read()
	{
		char* result;
		int cantidad_bytes = serial.available();
		if (cantidad_bytes > 0) {
			result = new char[cantidad_bytes];
			unsigned char lectura = 0;
			int i = 0;
			while(serial.readBytes(&lectura, 1) > 0) {
				result[i] = lectura;
				i++;
			}
			result[i] = 0;
		}
		else
		{
			result = new char[1];
			result[0] = '\0';
		}
		return result;
	}

	ofVec3f Arduino::getKinect3dCoordinates()
	{
		//angleMotor1 = motor de la base
		//angleMotor2 = motor del medio
		//angleMotor4 = motor de la punta
		//angleMotor8 = motor de mas de la punta
		//vamos a hallar las coordenadas x, y, z desde la base del brazo que est� apoyada
		//en el la mesa.
		//hallemos z
		//sen(alpha) * el largo del brazo (la mitad desde el centro)
		double y = sin((float)angleMotor2) * ARM_LENGTH;
		double z = cos((float)angleMotor2) * ARM_LENGTH;
		double x = sin((float)angleMotor1) * y;
		ofVec3f position = ofVec3f(float(x), float (y), float (z));
		return position;
	}

	void Arduino::setKinect3dCoordinates(float x, float y, float z)
	{
		int angleMotor2 = round(atan(z/x) * 180 / M_PI);			//el de la base, x no deberia ser 0 nunca
		int angleMotor1 = 0;
		if (y != 0) {
			if (y < 0) 
			{
				angleMotor1 = (int)round(asin(-y/ARM_LENGTH) * 180 / M_PI); // estaba mal, era el asin
			}
			else
			{
				angleMotor1 = -(int)round(asin(y/ARM_LENGTH) * 180 / M_PI); // estaba mal, era el asin
			}
		}
		sendMotor(angleMotor1, ID_MOTOR_1);
		sendMotor(angleMotor2, ID_MOTOR_2);
	}

	ofVec3f Arduino::setKinect3dCoordinates(ofVec3f position)
	{
		//wrapper para posicionar desde un ofVec3f
		ofVec3f closest_position = find_closest_point_to_sphere(position);
		setKinect3dCoordinates(closest_position.x, closest_position.y, closest_position.z);
		return closest_position;
	}

	ofVec3f Arduino::convert_3D_cart_to_spher(ofVec3f cart_point)
	{
		//devuelve un ofVec3f que por simplicidad:
		//x = r, y = phi, z = theta
		//http://www.thecubiclewarrior.com/post/5954842175/spherical-to-cartesian
		float r = sqrt(pow(cart_point.x, 2) + pow(cart_point.y, 2) + pow(cart_point.z, 2) );
		float phi = acos(cart_point.y/r); //z es mi y! es el angulo en el plano x, z
		float theta = atan2(cart_point.x, cart_point.z);//z es mi y!
		ofVec3f spher_point(r, phi, theta);
		return spher_point;
	}

	ofVec3f Arduino::convert_3D_spher_to_cart(ofVec3f spher_point)
	{
		//http://www.thecubiclewarrior.com/post/5954842175/spherical-to-cartesian
		float r = spher_point.x;
		float phi = spher_point.y;
		float theta = spher_point.z;

		float z = r*sin(phi)*cos(theta);
		float x = r*sin(phi)*sin(theta);
		float y = r*cos(phi);

		ofVec3f cart_point(x, y, z);
		return cart_point;
	}

	ofVec3f Arduino::find_closest_point_to_sphere(ofVec3f point)
	{
		//la l�gica es pasar el punto que viene a un punto en coordenadas esf�ricas
		//reducir el r y pasarlo nuevamente a coordenadas cartesianas.
		ofVec3f spher_orig_point = convert_3D_cart_to_spher(point);
		spher_orig_point.x = ARM_LENGTH;
		return convert_3D_spher_to_cart(spher_orig_point);
	}

	ofVec3f	Arduino::lookAt(ofVec3f point)
	{
		return NULL;
	}
	bool Arduino::isActive()
	{
		return IsFeatureArduinoActive();
	}
	ofVec3f	Arduino::lookingAt()
	{
		return NULL;
	}

	Eigen::Affine3f Arduino::getWorldTransformation()
	{
		//todas las matrices segun: http://pages.cs.brandeis.edu/~cs155/Lecture_07_6.pdf
		//CvMat* mat = cvCreateMat(4,4,CV_32FC1);
		//primero giramos seg�n el eje vertical (Y)
		//el angulo es el del motor inferior y la matriz de transformaci�n es:
		//[cos -sin  0  0]
		//[sen  cos  0  0]
		//[ 0    0   1  0]
		//[ 0    0   0  1]
		//se la pido a Eigen 8)
		Eigen::Vector3f axisY (0, 0, 1); //para Eigen el Z es nuestro Y ?
		Eigen::Affine3f rotationY;
		rotationY = Eigen::AngleAxis<float>(-angleMotor1, axisY);

		Eigen::Vector3f axisX (0, 1, 0);
		Eigen::Affine3f rotationX;
		rotationX = Eigen::AngleAxis<float>(-angleMotor2, axisX);

		Eigen::Vector3f axisZ (1, 0, 0);
		Eigen::Affine3f rotationZ;
		rotationZ = Eigen::AngleAxis<float>(-angleMotor4, axisZ);

		//con estas tres matrices tengo todas las rotaciones que preciso, ahora
		//preciso hallar la traslacion de altura donde esta la camara
		//y luego la traslacion a lo largo del brazo

		Eigen::Affine3f translationY;
		translationY = Eigen::Translation<float,3>(0, -5, 0);//puse 5 como un valor cualquiera, hay que ajustarlo

		//luego hay que correrlo el largo del brazo
		
		Eigen::Affine3f translationX;
		translationX = Eigen::Translation<float,3>(-ARM_LENGTH, 0, 0);//capaz se puede combinar con el anterior, no?

		Eigen::Affine3f composed_matrix;
		composed_matrix = rotationY * rotationX * rotationZ * translationY * translationX;
		return composed_matrix;


	}
}
