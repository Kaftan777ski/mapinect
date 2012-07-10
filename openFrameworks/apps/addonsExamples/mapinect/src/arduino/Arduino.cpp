#include "Arduino.h"

#include "Feature.h"
#include "ofxXmlSettings.h"
#include <direct.h> // for getcwd
#include "Plane3D.h"
#include "utils.h"
#include <pcl/registration/icp.h>

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

#define		ICP_CLOUD_DENSITY	8

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
	static float	MOTOR_ANGLE_OFFSET;
	static int		ANGLE_STEP;
	static int		MAX_ANGLE_1;
	static int		MIN_ANGLE_1;
	static int		MAX_ANGLE_2;
	static int		MIN_ANGLE_2;
	static int		MAX_ANGLE_4;
	static int		MIN_ANGLE_4;
	static int		MAX_ANGLE_8;
	static int		MIN_ANGLE_8;

	float			Arduino::ARM_LENGTH;
	float			Arduino::KINECT_HEIGHT;
	float			Arduino::MOTORS_HEIGHT;

	Eigen::Affine3f Arduino::worldTransformation  = Eigen::Affine3f();

	Arduino::Arduino()
	{
		angleMotor1 = 0;
		angleMotor2 = 0;
		angleMotor4 = 0;
		angleMotor8 = 0;
		armStoppedMoving = true;
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

			KINECT_HEIGHT = XML.getValue(ARDUINO_CONFIG "KINECT_HEIGHT", 0.0);
			MOTORS_HEIGHT = XML.getValue(ARDUINO_CONFIG "MOTORS_HEIGHT", 0.0);
			ARM_LENGTH = XML.getValue(ARDUINO_CONFIG "ARM_LENGTH", 0.0);

			MAX_ANGLE_1 = XML.getValue(ARDUINO_CONFIG "MAX_ANGLE_1", 0);
			MIN_ANGLE_1 = XML.getValue(ARDUINO_CONFIG "MIN_ANGLE_1", 0);
			MAX_ANGLE_2 = XML.getValue(ARDUINO_CONFIG "MAX_ANGLE_2", 0);
			MIN_ANGLE_2 = XML.getValue(ARDUINO_CONFIG "MIN_ANGLE_2", 0);
			MAX_ANGLE_4 = XML.getValue(ARDUINO_CONFIG "MAX_ANGLE_4", 0);
			MIN_ANGLE_4 = XML.getValue(ARDUINO_CONFIG "MIN_ANGLE_4", 0);
			MAX_ANGLE_8 = XML.getValue(ARDUINO_CONFIG "MAX_ANGLE_8", 0);
			MIN_ANGLE_8 = XML.getValue(ARDUINO_CONFIG "MIN_ANGLE_8", 0);
		}

		angleMotor1 = RESET_ANGLE1;
		angleMotor2 = RESET_ANGLE2;
		angleMotor4 = RESET_ANGLE4;
		angleMotor8 = RESET_ANGLE8;	// La posici�n inicial de este motor es mirando de costado. 

		if (!serial.setup(COM_PORT, 9600)) {
			cout << "Error en setup del Serial, puerto COM: " << COM_PORT << endl;
			//return false;
		}

		armMoving = true;

		EventManager::suscribe(this);

		sendMotor(angleMotor1, ID_MOTOR_1);
		sendMotor(angleMotor2, ID_MOTOR_2);
		sendMotor(angleMotor4, ID_MOTOR_4);
		sendMotor(angleMotor8, ID_MOTOR_8);

		calculateWorldTransformation(angleMotor1,angleMotor2,angleMotor4,angleMotor8);

		posicion = getKinect3dCoordinates();
		mira = lookingAt();

		serial.enumerateDevices();
		
		armStoppedMoving = false;

		return true;
	}

	void Arduino::exit() {
		CHECK_ACTIVE;
		if (serial.available()){
			serial.close();
		}
	}

	void Arduino::update() {
		CHECK_ACTIVE;

		if (armStoppedMoving)
		{
			armStoppedMoving = false;

			if (!(cloudBeforeMoving.get() == NULL)) 
			{

				cloudAfterMoving = getCloud(ICP_CLOUD_DENSITY);

				// Apply ICP

				pcl::PointCloud<PCXYZ>::Ptr beforeMoving (new pcl::PointCloud<PCXYZ>(*cloudBeforeMoving.get()));
				pcl::PointCloud<PCXYZ>::Ptr afterMoving  (new pcl::PointCloud<PCXYZ>(*cloudAfterMoving.get()));

				pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
				icp.setInputCloud(beforeMoving);
				icp.setInputTarget(afterMoving);

				// Set the max correspondence distance to 5cm (e.g., correspondences with higher distances will be ignored)
				//icp.setMaxCorrespondenceDistance (0.05);
				// Set the maximum number of iterations (criterion 1)
				//icp.setMaximumIterations (5);
				// Set the transformation epsilon (criterion 2)
				//icp.setTransformationEpsilon (1e-8);
				// Set the euclidean distance difference epsilon (criterion 3)
				//icp.setEuclideanFitnessEpsilon (1);

				pcl::PointCloud<pcl::PointXYZ> Final;
				icp.align(Final);
				if (icp.hasConverged())
				{
					cout << "ICP has converged with fitness score: " << icp.getFitnessScore() << endl;
					Eigen::Affine3f newTransf (icp.getFinalTransformation());

					//Combine new transformation with estimated transf
					worldTransformation = newTransf.inverse() * worldTransformation  ;
				}

			}
		}

	}

	void Arduino::draw() {
		CHECK_ACTIVE;
	}

	void Arduino::keyPressed (int key) {
		CHECK_ACTIVE;

		ofVec3f centroidePrueba(0.3188, 0.2454, 1.0842);

		float sin8 = 0.12533;
		float sin15 = 0.2588;
		float sin30 = 0.5;
		float sin45 = 0.7071;
		float cos8 = 0.9921;
		float cos15 = 0.9659;
		float cos30 = 0.866;
		float cos45 = 0.7071;
		switch (key)
		{
			case '9':
				lookAt(centroidePrueba);
				break;
			case '0':
				setArm3dCoordinates(ofVec3f(Arduino::ARM_LENGTH, 0, 0)); 
				break;
			case '1':
				//AngleMotor1 = -15
				setArm3dCoordinates(ofVec3f(Arduino::ARM_LENGTH*cos15, -Arduino::ARM_LENGTH*sin15, 0)); 
				break;
			case '2':
				//AngleMotor2 = 15
				setArm3dCoordinates(ofVec3f(Arduino::ARM_LENGTH*cos15, 0, Arduino::ARM_LENGTH*sin15)); 
				break;
			case '3':
				//AngleMotor8 > 90
				setArm3dCoordinates(ofVec3f(Arduino::ARM_LENGTH, 0, 0));
				break;
			case '4':
				//AngleMotor4 < 0 
				setArm3dCoordinates(ofVec3f(Arduino::ARM_LENGTH, 0, 0));
				break;
			case '5':
				//AngleMotor1 = -8
				setArm3dCoordinates(ofVec3f(Arduino::ARM_LENGTH*cos8, -Arduino::ARM_LENGTH*sin8, 0)); 
				break;
			case '.':
				// For testing - Vero
				armStoppedMoving = true;
				armMoving = false;
				break;
		}
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
		else if (key == 'z')
		{
			setArm3dCoordinates(ofVec3f(ARM_LENGTH, -0.10, 0.10));
		}
		else if (key == 'a')
		{
			setArm3dCoordinates(ofVec3f(ARM_LENGTH, 0, 0));
		}
		else if (key == 'x')
		{
			lookAt(ofVec3f(0.35, -0.16, 0.15));
		}
		else if (key == 'c')
		{
			lookAt(ofVec3f(0.35, -0.13, 0.10));
		}
		else if (key == 's')
		{
			lookAt(ofVec3f(0.33, -KINECT_HEIGHT-MOTORS_HEIGHT, 0.1));
		}		

	}


	void Arduino::reset()
	{
		CHECK_ACTIVE;

		armMoving = true;

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

		calculateWorldTransformation(angleMotor1,angleMotor2,angleMotor4,angleMotor8);

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
		char id_char = (char) id;
		serial.writeByte(id_char);
		serial.writeByte(value);
	}

	signed int* Arduino::motorAngles() const
	{
		signed int* result = new signed int[4];
		result[0] = angleMotor1;
		result[1] = angleMotor2;
		result[2] = angleMotor4;
		result[3] = angleMotor8;
		return result;
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
		Eigen::Vector3f kinectPos (0.0, 0.0, 0.0);		// Posicion inicial, en Sist. de Coord Local del Kinect
		kinectPos = getWorldTransformation() * kinectPos;
		return ofVec3f(kinectPos.x(), kinectPos.y(), kinectPos.z());		
		//return posicion;	

/*		//angleMotor1 = motor de la base
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
*/	}

	void Arduino::setArm3dCoordinates(float x, float y, float z)
	{
		armMoving = true;

		// Get cloud before moving arm
		cloudBeforeMoving = getCloud(ICP_CLOUD_DENSITY);

		// Setear las coordenadas de la posici�n donde estar� el motor8 (el de m�s abajo del Kinect)
		//		en coordenadas de mundo
		angleMotor2 = round(atan(z/x) * 180.0f / M_PI);			//el de la base, x no deberia ser 0 nunca
		if (y != 0) {
			if (y > 0)
			{
				angleMotor1 = (int)round(asin(y/ARM_LENGTH) * 180.0f / M_PI); // estaba mal, era el asin
			}
			else
			{
				angleMotor1 = -(int)round(asin(-y/ARM_LENGTH) * 180.0f / M_PI); // estaba mal, era el asin
			}
		}
		if (!inRange(angleMotor4, MIN_ANGLE_1, MAX_ANGLE_1))
		{
			return;
		}

		if (!inRange(angleMotor2, MIN_ANGLE_2, MAX_ANGLE_2))
		{
			return;
		}

		sendMotor(angleMotor1, ID_MOTOR_1);
		sendMotor(angleMotor2, ID_MOTOR_2);

		calculateWorldTransformation(angleMotor1,angleMotor2,angleMotor4,angleMotor8);

		posicion = getKinect3dCoordinates(); //ofVec3f(x, y, z);
	}

	ofVec3f Arduino::setArm3dCoordinates(const ofVec3f& position)
	{
		// Setear las coordenadas de la posici�n donde estar� el motor8 (el de m�s abajo del Kinect)
		// en coordenadas de mundo
		// wrapper para posicionar desde un ofVec3f
		ofVec3f bestFit = bestFitForArmSphere(position);
		setArm3dCoordinates(bestFit.x, bestFit.y, bestFit.z);
		return bestFit;
	}

	ofVec3f Arduino::bestFitForArmSphere(const ofVec3f& p)
	{
		//la l�gica es pasar el punto que viene a un punto en coordenadas esf�ricas
		//reducir el r y pasarlo nuevamente a coordenadas cartesianas.
		Line3D armLine(ofVec3f(0, 0, 0), p);
		return armLine.calculateValue(Arduino::ARM_LENGTH / armLine.segmentLength());
	}

	ofVec3f	Arduino::lookAt(const ofVec3f& point)
	{
		armMoving = true;

		// Get cloud before moving arm
		cloudBeforeMoving = getCloud(ICP_CLOUD_DENSITY);

		ofVec3f miraHorizonte (ARM_LENGTH + 0.10, - KINECT_HEIGHT - MOTORS_HEIGHT, 0.0);
		ofVec3f posInicialKinect (ARM_LENGTH, - KINECT_HEIGHT - MOTORS_HEIGHT, 0.0);
		//posicion = getKinect3dCoordinates();

		//TODO: tener el cuenta la traslacion del grueso de los motores de la punta
		//posicion = donde se encuentra ubicado
		//mira = donde estoy mirando ATM
		Eigen::Vector3f axisY (0, 1, 0);
		Eigen::Affine3f rotationY;
		float angleMotor2Rad = ofDegToRad(angleMotor2); // Motor de abajo del brazo, con la varilla "vertical"
		rotationY = Eigen::AngleAxis<float>(angleMotor2Rad, axisY);				// Debe ser positivo

		Eigen::Vector3f axisZ (0, 0, 1);
		Eigen::Affine3f rotationZ;
		float angleMotor1Rad = ofDegToRad(angleMotor1);	// Motor que mueve la varilla "horizontal"
		rotationZ = Eigen::AngleAxis<float>(-angleMotor1Rad, axisZ);			// Debe ser negativo

		Eigen::Affine3f composed_matrix;
		composed_matrix = rotationY * rotationZ;
		
		Eigen::Vector3f e_point = Eigen::Vector3f(point.x, point.y, point.z);
		Eigen::Vector3f e_mira = Eigen::Vector3f(miraHorizonte.x, miraHorizonte.y, miraHorizonte.z);
		Eigen::Vector3f e_posicion = Eigen::Vector3f(posInicialKinect.x, posInicialKinect.y, posInicialKinect.z);

		Eigen::Vector3f et_point = composed_matrix * e_point;
		Eigen::Vector3f et_mira = /*composed_matrix * */ e_mira;
		Eigen::Vector3f et_posicion = /*composed_matrix * */ e_posicion;

		ofVec3f t_point = ofVec3f(et_point.x(), et_point.y(), et_point.z());
		ofVec3f t_mira = ofVec3f(et_mira.x(), et_mira.y(), et_mira.z());
		ofVec3f t_posicion = ofVec3f(et_posicion.x(), et_posicion.y(), et_posicion.z());

		ofVec3f origen = ofVec3f(0, 0, 0);
		ofVec3f eje_y = ofVec3f(0, 1, 0);
		Plane3D horizontal = Plane3D(origen, eje_y); //lo defino con el plano que xz y normal y

		ofVec3f pht_point = horizontal.project(t_point);
		ofVec3f pht_mira = horizontal.project(t_mira);
		ofVec3f pht_posicion = horizontal.project(t_posicion);

		ofVec3f hv_mira = pht_mira - pht_posicion;
		ofVec3f hv_point = pht_point - pht_posicion;

		float angulo_h = 0;
		
		// Para monitorear los valores

		ofVec3f h_normal = horizontal.getNormal();
/*		*****FOR DEBUG*****		*/
/*		float hv_mira_len = hv_mira.length();
		float hv_point_len = hv_point.length();
		float hv_mira_dot = hv_mira.dot(h_normal);
		float h_normal_len = h_normal.length();
		float hv_point_dot = hv_point.dot(h_normal);
*/		//
		if (!(abs(hv_mira.length()) <= MATH_EPSILON || abs(hv_point.length()) <= MATH_EPSILON	
				|| (abs(hv_mira.dot(h_normal) - hv_mira.length()*h_normal.length()) <= MATH_EPSILON)
				|| (abs(hv_point.dot(h_normal) - hv_point.length()*h_normal.length()) <= MATH_EPSILON) )) {
				//if (length de alguno de los vectores < MATH_EPSILON || alguno de los vectores es "casi" paralelo a la normal del plano)
				angulo_h = hv_mira.angle(hv_point);//motor 8
				if (point.z < 0)
				{
					angulo_h *= -1;
				}
		}		 

		ofVec3f eje_z = ofVec3f(0, 0, 1);
		eje_z = eje_z.rotate(-angulo_h, eje_y);
		Plane3D vertical = Plane3D(t_posicion, eje_z);

		ofVec3f mira_trans = ofVec3f(t_mira.x - posInicialKinect.x, t_mira.y - posInicialKinect.y, t_mira.z - posInicialKinect.z); 
		mira_trans = mira_trans.rotate(-angulo_h, eje_y);
		t_mira = ofVec3f(mira_trans.x + posInicialKinect.x, mira_trans.y + posInicialKinect.y, mira_trans.z + posInicialKinect.z);

		ofVec3f pvt_point = vertical.project(t_point);
		ofVec3f pvt_mira = vertical.project(t_mira);
		ofVec3f pvt_posicion = vertical.project(t_posicion);

		ofVec3f vv_mira = pvt_mira - pvt_posicion;
		ofVec3f vv_point = pvt_point - pvt_posicion;

		float angulo_v = 0;
		ofVec3f v_normal = vertical.getNormal();
/*		*****FOR DEBUG*****		*/
/*		float vv_mira_len = vv_mira.length();
		float vv_point_len = vv_point.length();
		float vv_mira_dot = vv_mira.dot(v_normal);
		float v_normal_len = v_normal.length();
		float vv_point_dot = vv_point.dot(v_normal);
*/		//
		if (!(abs(vv_mira.length()) <= MATH_EPSILON || abs(vv_point.length()) <= MATH_EPSILON	
				|| (abs(vv_mira.dot(v_normal) - vv_mira.length()*v_normal.length()) <= MATH_EPSILON)
				|| (abs(vv_point.dot(v_normal) - vv_point.length()*v_normal.length()) <= MATH_EPSILON) )) {
				//if (length de alguno de los vectores < MATH_EPSILON || alguno de los vectores es "casi" paralelo a la normal del plano)	
				angulo_v = vv_mira.angle(vv_point);//motor 4
				if (point.y > (- KINECT_HEIGHT - MOTORS_HEIGHT) )
				{
					angulo_v *= -1;
				}
		}		 	

		angleMotor4 = angulo_v;
		angleMotor8 = angulo_h;

		//mira_actual = point;
		mira = point;

		/*if (!inRange(angleMotor4, MIN_ANGLE_4, MAX_ANGLE_4))
		{
			return NULL;
		}
		if (!inRange(angleMotor8, MIN_ANGLE_8, MAX_ANGLE_8))
		{
			return NULL;
		}*/
		sendMotor(angleMotor4, ID_MOTOR_4);
		sendMotor(angleMotor8, ID_MOTOR_8);

		calculateWorldTransformation(angleMotor1,angleMotor2,angleMotor4,angleMotor8);

		return NULL;
	}

	bool Arduino::isActive()
	{
		return IsFeatureArduinoActive();
	}

	ofVec3f	Arduino::lookingAt()
	{
		Eigen::Vector3f kinectMira (0.0, 0.0, 0.10);		// Mira inicial, en Sist. de Coord Local del Kinect
		kinectMira = getWorldTransformation() * kinectMira;
		return ofVec3f(kinectMira.x(), kinectMira.y(), kinectMira.z());	
		//return NULL;
		//return mira;
	}

	Eigen::Affine3f Arduino::getWorldTransformation()
	{
		return Arduino::worldTransformation;
	}

	Eigen::Affine3f Arduino::calculateWorldTransformation(float angle1, float angle2, float angle4, float angle8)
	{
		float angleMotor1Rad = ofDegToRad(angle1);		// Motor que mueve la varilla "horizontal"
		float angleMotor2Rad = ofDegToRad(angle2);		// Motor de abajo del brazo, con la varilla "vertical"
		float angleMotor4Rad = ofDegToRad(angle4);		// Motor de los de la punta, el de m�s arriba, sobre el que est� enganchado la base del Kinect
		float angleMotor8Rad = ofDegToRad(angle8 - 90);	// Motor de los de la punta, el de m�s abajo. La posici�n inicial de referencia ser� 90 grados.

		//todas las matrices segun: http://pages.cs.brandeis.edu/~cs155/Lecture_07_6.pdf
		//CvMat* mat = cvCreateMat(4,4,CV_32FC1);
		//primero giramos seg�n el eje vertical (Y)
		//el angulo es el del motor inferior y la matriz de transformaci�n es:
		//[cos -sin  0  0]
		//[sen  cos  0  0]
		//[ 0    0   1  0]
		//[ 0    0   0  1]		// Coment Vero: Ojo que esta matriz es una rotaci�n seg�n el eje Z, no el Y

		Eigen::Vector3f axisX (1, 0, 0);
		Eigen::Vector3f axisY (0, 1, 0);
		Eigen::Vector3f axisZ (0, 0, 1);

		Eigen::Affine3f rotationY;
		rotationY = Eigen::AngleAxis<float>(-angleMotor2Rad, axisY);
		
		Eigen::Affine3f rotationZ;
		rotationZ = Eigen::AngleAxis<float>(angleMotor1Rad, axisZ);

		Eigen::Affine3f translationX;
		translationX = Eigen::Translation<float, 3>(ARM_LENGTH, 0, 0);//capaz se puede combinar con el anterior, no?

		Eigen::Affine3f rotationY2;
		rotationY2 = Eigen::AngleAxis<float>(-angleMotor8Rad, axisY);

		Eigen::Affine3f translationY;
		translationY = Eigen::Translation<float, 3>(0, -MOTORS_HEIGHT, 0);

		Eigen::Affine3f rotationX;
		rotationX = Eigen::AngleAxis<float>(angleMotor4Rad, axisX);

		Eigen::Affine3f translationY2;
		translationY2 = Eigen::Translation<float, 3>(0, -KINECT_HEIGHT, 0);

		//con estas tres matrices tengo todas las rotaciones que preciso, ahora
		//preciso hallar la traslacion de altura donde esta la camara
		//y luego la traslacion a lo largo del brazo
		
		Eigen::Affine3f composed_matrix;
		composed_matrix = rotationY * (rotationZ *  (translationX * (rotationY2 * (translationY * (rotationX * translationY2)))));
		//composed_matrix = translationY2 * rotationX * translationY * rotationY2 * translationX * rotationZ * rotationY;

		/*
			Pruebas para verificar que se est�n calculando bien las transformaciones
		*/

		/* Las siguientes pruebas utilizan los valores de �ngulos:
				angleMotor1 = -45;
				angleMotor2 = 0;
				angleMotor4 = 0;
				angleMotor8 = 90;		*/

		//Eigen::Vector3f ejemplo (0.0, 0.0, 0.0); //En Sist de Coordenadas Local, del Kinect
		// Deber�a dar como resultado: X = 0.14, Y = -0.35, Z = 0	=> Dio OK

		// Eigen::Vector3f ejemplo (0.0, MOTORS_HEIGHT + KINECT_HEIGHT, 0.0); //En Sist de Coordenadas Local, del Kinect
		// Deber�a dar como resultado: X = 0.25, Y = -0.25, Z = 0	=> Dio OK

		/* Las siguientes pruebas utilizan los valores de �ngulos:
				angleMotor1 = -45;
				angleMotor2 = 0;
				angleMotor4 = -45;
				angleMotor8 = 90;		*/

		//Eigen::Vector3f ejemplo (0.0, 0.0, 0.0); //En Sist de Coordenadas Local, del Kinect
		// Dio como resultado: X = 0.14, Y = -0.33, Z = 0.07	=> Parece OK

		//Eigen::Vector3f ejemplo (0.1, 0.1, 0.1); //En Sist de Coordenadas Local, del Kinect
		// Dio como resultado: X = 0.32, Y = -0.30, Z = 0.07	=> Parece razonable...

		/* Las siguientes pruebas utilizan los valores de �ngulos:
				angleMotor1 = 0;
				angleMotor2 = 0;
				angleMotor4 = -45;
				angleMotor8 = 45;		*/

		//Eigen::Vector3f ejemplo (0.0, 0.0, 0.0); //En Sist de Coordenadas Local, del Kinect
		// Dio como resultado: X = 0.40, Y = -0.13, Z = 0.05	=> Parece OK

		/* Las siguientes pruebas utilizan los valores de �ngulos:
				angleMotor1 = 0;
				angleMotor2 = 45;
				angleMotor4 = 0;
				angleMotor8 = 90;		*/

		//Eigen::Vector3f ejemplo (0.0, 0.0, 0.0); //En Sist de Coordenadas Local, del Kinect
		// Dio como resultado: X = 0.24, Y = -0.16, Z = 0.24	=> Parece OK

		/*
		ejemplo = translationY2 * ejemplo;
		ejemplo = rotationX * ejemplo;
		ejemplo = translationY * ejemplo;
		ejemplo = rotationY2 * ejemplo;
		ejemplo = translationX * ejemplo;
		ejemplo = rotationZ * ejemplo;
		ejemplo = rotationY * ejemplo; 

		Eigen::Vector3f ej (0.0, 0.0, 0.0);
		ej = composed_matrix * ej;
		*/

		worldTransformation = composed_matrix;

		return composed_matrix;

	}

	void Arduino::objectUpdated(const IObjectPtr& object)
	{
		if (object->getId() == id_object_to_follow)
		{
			if (center_of_following_object.distance(object->getCenter()) > 0.05)
			{
				lookAt(object->getCenter());
			}
		}
	}

	
	void Arduino::followObject(const IObjectPtr& object)
	{
		id_object_to_follow = object->getId();
		center_of_following_object = object->getCenter();
		//lookAt(object->getCenter());
	}

}
