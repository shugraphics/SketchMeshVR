#include <stdlib.h>
#include <iostream>
#include <igl/readOFF.h>
#include <igl/viewer/Viewer.h>
#include <igl/vertex_triangle_adjacency.h>
#include <igl/adjacency_list.h>
#include <igl/per_face_normals.h>
#include <igl/per_vertex_normals.h>
#include <igl/per_corner_normals.h>
#include <igl/facet_components.h>
#include <igl/jet.h>
#include <igl/barycenter.h>
#include <cmath>
#include <igl/triangle_triangle_adjacency.h>
#include "SketchMesh.h"
#include "Stroke.h"

/*** insert any libigl headers here ***/

using namespace std;
//using System.Diagnostics;
using Viewer = igl::viewer::Viewer;

// Vertex array, #V x3
Eigen::MatrixXd V;
// Face array, #F x3
Eigen::MatrixXi F;
// Per-face normal array, #F x3
Eigen::MatrixXd FN;
// Per-vertex normal array, #V x3
Eigen::MatrixXd VN;
// Per-corner normal array, (3#F) x3
Eigen::MatrixXd CN;
// Vectors of indices for adjacency relations
std::vector<std::vector<int> > VF, VFi, VV;
// Integer vector of component IDs per face, #F x1
Eigen::VectorXi cid;
// Per-face color array, #F x3
Eigen::MatrixXd component_colors_per_face;

//Mouse interaction
enum ToolMode{DRAW, ADD, CUT, EXTRUDE, PULL, REMOVE, CHANGE, SMOOTH, NAVIGATE, NONE};
ToolMode tool_mode = NAVIGATE;
bool skip_standardcallback = false;
int down_mouse_x = -1, down_mouse_y = -1;

//For selecting vertices
std::unique_ptr<Stroke> _stroke;

bool callback_key_down(Viewer& viewer, unsigned char key, int modifiers) {

	if (key == '1') {
		//viewer.data.clear();
		//viewer.data.set_mesh(V, F);
	}

	if (key == 'D') { //use capital letters
		//Draw initial curve/mesh
		tool_mode = DRAW;
		cout << "tes" << endl;
	}
	if (key == 'P') {
		tool_mode = PULL;
	}
	if (key == 'N') {
		//Use navigation
		tool_mode = NAVIGATE;
	}

	//viewer.ngui->refresh(); //TODO: is this needed?
	return true;
}

bool callback_mouse_down(Viewer& viewer, int button, int modifier) {
	if (button == (int)Viewer::MouseButton::Right) {
		return false;
	}
	down_mouse_x = viewer.current_mouse_x;
	down_mouse_y = viewer.current_mouse_y;

	if (tool_mode == DRAW) { //Creating the first curve/mesh
		_stroke->strokeAddSegment(down_mouse_x, down_mouse_y);
		skip_standardcallback = true;
	}
	else if (tool_mode == PULL) { //Dragging an existing curve
		skip_standardcallback = true;
	}
	else if (tool_mode == NAVIGATE) { //Navigate through the screen
		skip_standardcallback = false; //We do want to use the navigation functionality
	}


	return skip_standardcallback; //Will make sure that we use standard navigation responses if we didn't do special actions and vice versa
}


bool callback_mouse_move(Viewer& viewer, int mouse_x, int mouse_y) {
	if (!skip_standardcallback) {
		return false;
	}
	if (tool_mode == DRAW && CONSOLE_MOUSE_DOWN) {
		_stroke->strokeAddSegment(mouse_x, mouse_y);
		return true;
	}
	return false;
}

//TODO: make callback for this in viewer, like in exercise 5 of shapemod
bool callback_load_mesh(Viewer& viewer, string filename)
{
	igl::readOFF(filename, V, F);
	viewer.data.clear();
	viewer.data.set_mesh(V, F);
	viewer.data.compute_normals();
	viewer.core.align_camera_center(viewer.data.V);

	//Init stroke selector
	
	_stroke = std::unique_ptr<Stroke>(new Stroke(V, F, viewer));
	std::cout << filename.substr(filename.find_last_of("/") + 1) << endl;
	return true;
}

int main(int argc, char *argv[]) {
	// Show the mesh
	Viewer viewer;
	viewer.callback_key_down = callback_key_down;
	viewer.callback_mouse_down = callback_mouse_down;
	viewer.callback_mouse_move = callback_mouse_move;
	//viewer.callback_load_mesh = callback_load_mesh;
	

	if (argc == 2)
	{
		// Read mesh
		igl::readOFF(argv[1], V, F);
		callback_load_mesh(viewer, argv[1]);
	}
	else
	{
		// Read mesh
		igl::readOFF("../data/cube.off", V, F);
		callback_load_mesh(viewer, "../data/cube.off");
	}
	Eigen::Vector3d m = V.colwise().minCoeff();
	Eigen::Vector3d M = V.colwise().maxCoeff();

	// Corners of the bounding box
	Eigen::MatrixXd V_box(8, 3);
	V_box <<
		m(0), m(1), m(2),
		M(0), m(1), m(2),
		M(0), M(1), m(2),
		m(0), M(1), m(2),
		m(0), m(1), M(2),
		M(0), m(1), M(2),
		M(0), M(1), M(2),
		m(0), M(1), M(2);

	cout << V_box.row(0) << endl;
	// Edges of the bounding box
	Eigen::MatrixXi E_box(12, 2);
	E_box <<
		0, 1,
		1, 2,
		2, 3,
		3, 0,
		4, 5,
		5, 6,
		6, 7,
		7, 4,
		0, 4,
		1, 5,
		2, 6,
		7, 3;
	viewer.data.add_stroke_points(V_box);

	callback_key_down(viewer, '1', 0);

	
	viewer.core.align_camera_center(V);
	viewer.launch();
}
