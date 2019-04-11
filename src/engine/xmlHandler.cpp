#include <xmlHandler.h>


string cur_dir;


/**
Função que interpreta um cen‡rio gr‡fico em XML
*/

int readXML(const char *filename, vector<Group>* groups){
	cur_dir = getDirectory(filename);	
	XMLDocument doc;
	XMLError error = doc.LoadFile(filename);
	if (error != XML_SUCCESS) { printf("Error: %i\n", error); return error; }

	XMLNode *scene = doc.FirstChild();
	if (scene == nullptr) return XML_ERROR_FILE_READ_ERROR;

	XMLElement *groups_xml = scene->FirstChildElement("group");

	while (groups_xml != nullptr) {
		vector<Operation*> ops;
		vector<Figure> fig;
		vector<Group> subGroups;
		readGroup(groups_xml, &fig, &ops, &subGroups);
		
		Group group; 
		group.set_values(fig, ops, subGroups);
		groups->push_back(group);

		groups_xml = groups_xml->NextSiblingElement("group");
	}

	return XML_SUCCESS;
}


int readGroup(XMLElement* element, vector<Figure>* fig, vector<Operation*>* ops, vector<Group>* subGroups) {
	XMLElement* child;
	int flag = 0; // PODEMOS RETIRAR ESTAR FLAG NÃO?
	int nr_points = 0;
	for (child = element->FirstChildElement(); child != NULL && flag == 0; child = child->NextSiblingElement())	{
		vector<Figure> aux;
		vector<Operation*> aux2;
		vector<Group> aux3;
		vector<Point> pts;
		Group g;

        
		float time, x = 1, y = 1, z = 1; // Inicializar a 1 por causa do scale. Caso nao consiga ler Y, Y=0 e tem que ser 1. Daí usar QueryFloatSttribute tambem
		double angle = 0;
		switch (hashF((char*)child->Value())){
			case TRANSLATE:
				if (child->FindAttribute("time") != NULL) {
					time = child->FloatAttribute("time");
					printf("time = %f \n", time);
					XMLElement* point;
					for (point = child->FirstChildElement(); point != NULL; point = point->NextSiblingElement()) {
						x = point->FloatAttribute("X");
						y = point->FloatAttribute("Y");
						z = point->FloatAttribute("Z");

						pts.push_back(*new Point(x, y, z));
						printf("Ponto %d = (%f, %f, %f) \n", nr_points, x, y, z);
						nr_points++; // para verificar no fim se são 4+
					}

					if (nr_points < 4) return -1;

					(*ops).push_back(new DynamicTranslate(time, pts));
				}
				else {
					x = child->FloatAttribute("X");
					y = child->FloatAttribute("Y");
					z = child->FloatAttribute("Z");

					(*ops).push_back(new Translate(x, y, z));

					printf("Static Translate = (%f, %f, %f) \n", x, y, z);
				}
				
				break;

			case ROTATE:
				if (child->FindAttribute("time") != NULL) {
					time = child->FloatAttribute("time");
					x = child->FloatAttribute("axisX");
					y = child->FloatAttribute("axisY");
					z = child->FloatAttribute("axisZ");

					(*ops).push_back(new DynamicRotate(time, x, y, z));
					printf("Dynamic Rotate = (%f, %f, %f, %f) \n", time, x, y, z);

				}
				else {
					angle = child->FloatAttribute("angle");
					x = child->FloatAttribute("axisX");
					y = child->FloatAttribute("axisY");
					z = child->FloatAttribute("axisZ");

					(*ops).push_back(new Rotate(time, x, y, z));
					printf("Static Rotate = (%f, %f, %f, %f) \n", angle, x, y, z);
				}
				
				break;

			case SCALE:
				child->QueryFloatAttribute("X", &x);
				child->QueryFloatAttribute("Y", &y);
				child->QueryFloatAttribute("Z", &z);

				(*ops).push_back(new Scale(x, y, z));
				break;

			case MODELS:
				readModels(child, fig);
				break;

			case GROUP:
				readGroup(child, &aux, &aux2, &aux3);
				g.set_values(aux,aux2,aux3);
				(*subGroups).push_back(g);
				break;

			default:
				break;
		}
	}

	return XML_SUCCESS;
}


int readModels(XMLElement* models, vector<Figure>* fig){
	if (models != nullptr) {
		XMLElement *model = models->FirstChildElement("model");
		while (model != nullptr) {
			const char * fileName = nullptr;
			fileName = model->Attribute("file");

			if (fileName == nullptr) return XML_ERROR_PARSING_ATTRIBUTE;

			string fname(fileName);
			string path = cur_dir+fname;
			Figure f = getFigure(path);
			(*fig).push_back(f);

			model = model->NextSiblingElement("model");
		}

		models = models->NextSiblingElement("models");
		return XML_SUCCESS;
	}
	return XML_NO_TEXT_NODE;
}



string getDirectory(const string& name){
    size_t pos = name.find_last_of("/\\");
    string path = name.substr(0, pos+1);
    return ( pos != string::npos)? path : "";
}