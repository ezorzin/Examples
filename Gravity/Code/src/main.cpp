/// @file     main.cpp
/// @author   Erik ZORZIN
/// @date     11MAR2021
/// @brief    Central gravitational potantial simulated attractor in a 3D continuum body.

#define INTEROP       true                                                                          // "true" = use OpenGL-OpenCL interoperability.
#define SX            800                                                                           // Window x-size [px].
#define SY            600                                                                           // Window y-size [px].
#define NAME          "Neutrino - Gravity"                                                          // Window name.
#define ORBX          0.0f                                                                          // x-axis orbit initial rotation.
#define ORBY          0.0f                                                                          // y-axis orbit initial rotation.
#define PANX          0.0f                                                                          // x-axis pan initial translation.
#define PANY          0.0f                                                                          // y-axis pan initial translation.
#define PANZ          -2.0f                                                                         // z-axis pan initial translation.

#ifdef __linux__
  #define SHADER_HOME "../../Gravity/Code/shader/"                                                  // Linux OpenGL shaders directory.
  #define KERNEL_HOME "../../Gravity/Code/kernel/"                                                  // Linux OpenCL kernels directory.
  #define GMSH_HOME   "../../Gravity/Code/mesh/"                                                    // Linux GMSH mesh directory.
#endif

#ifdef WIN32
  #define SHADER_HOME "..\\..\\Gravity\\Code\\shader\\"                                             // Windows OpenGL shaders directory.
  #define KERNEL_HOME "..\\..\\Gravity\\Code\\kernel\\"                                             // Windows OpenCL kernels directory.
  #define GMSH_HOME   "..\\..\\Gravity\\Code\\mesh\\"                                               // Linux GMSH mesh directory.
#endif

#define SHADER_VERT   "voxel_vertex.vert"                                                           // OpenGL vertex shader.
#define SHADER_GEOM   "voxel_geometry.geom"                                                         // OpenGL geometry shader.
#define SHADER_FRAG   "voxel_fragment.frag"                                                         // OpenGL fragment shader.
#define KERNEL_1      "thekernel1.cl"                                                               // OpenCL kernel source.
#define KERNEL_2      "thekernel2.cl"                                                               // OpenCL kernel source.
#define UTILITIES     "utilities.cl"                                                                // OpenCL kernel source.
#define MESH          "gravity.msh"                                                                 // GMSH mesh.

// INCLUDES:
#include "nu.hpp"                                                                                   // Neutrino header file.

int main ()
{
  // INDEXES:
  size_t                           i;                                                               // Index [#].
  size_t                           j;                                                               // Index [#].
  size_t                           j_min;                                                           // Index [#].
  size_t                           j_max;                                                           // Index [#].

  // MOUSE PARAMETERS:
  float                            ms_orbit_rate  = 1.0f;                                           // Orbit rotation rate [rev/s].
  float                            ms_pan_rate    = 5.0f;                                           // Pan translation rate [m/s].
  float                            ms_decaytime   = 1.25f;                                          // Pan LP filter decay time [s].

  // GAMEPAD PARAMETERS:
  float                            gmp_orbit_rate = 1.0f;                                           // Orbit angular rate coefficient [rev/s].
  float                            gmp_pan_rate   = 1.0f;                                           // Pan translation rate [m/s].
  float                            gmp_decaytime  = 1.25f;                                          // Low pass filter decay time [s].
  float                            gmp_deadzone   = 0.3f;                                           // Gamepad joystick deadzone [0...1].

  // OPENGL:
  nu::opengl*                      gl             = new nu::opengl (
                                                                    NAME,
                                                                    SX,
                                                                    SY,
                                                                    ORBX,
                                                                    ORBY,
                                                                    PANX,
                                                                    PANY,
                                                                    PANZ
                                                                   );                               // OpenGL context.
  nu::shader*                      S              = new nu::shader ();                              // OpenGL shader program.

  // OPENCL::
  nu::opencl*                      cl             = new nu::opencl (NU_GPU);                        // OpenCL context.
  nu::kernel*                      K1             = new nu::kernel ();                              // OpenCL kernel array.
  nu::kernel*                      K2             = new nu::kernel ();                              // OpenCL kernel array.
  nu::float4*                      color          = new nu::float4 (0);                             // Color [].
  nu::float4*                      position       = new nu::float4 (1);                             // Position [m].
  nu::float4*                      velocity       = new nu::float4 (2);                             // Velocity [m/s].
  nu::float4*                      acceleration   = new nu::float4 (3);                             // Acceleration [m/s^2].
  nu::float4*                      position_int   = new nu::float4 (4);                             // Position (intermediate) [m].
  nu::float4*                      velocity_int   = new nu::float4 (5);                             // Velocity (intermediate) [m/s].
  nu::float1*                      radius         = new nu::float1 (6);                             // Nucleus radius [m].
  nu::float1*                      stiffness      = new nu::float1 (7);                             // Stiffness.
  nu::float1*                      resting        = new nu::float1 (8);                             // Resting.
  nu::float1*                      friction       = new nu::float1 (9);                             // Friction.
  nu::float1*                      mass           = new nu::float1 (10);                            // Mass.
  nu::int1*                        central        = new nu::int1 (11);                              // Central nodes.
  nu::int1*                        neighbour      = new nu::int1 (12);                              // Neighbour.
  nu::int1*                        offset         = new nu::int1 (13);                              // Offset.
  nu::int1*                        freedom        = new nu::int1 (14);                              // Freedom.
  nu::float1*                      dt             = new nu::float1 (15);                            // Time step [s].

  // MESH:
  nu::mesh*                        gravity        = new nu::mesh (
                                                                  std::string (
                                                                               GMSH_HOME
                                                                              ) + std::string (
                                                                                               MESH
                                                                                              )
                                                                 );                                 // Mesh cloth.
  size_t                           nodes;                                                           // Number of nodes.
  size_t                           elements;                                                        // Number of elements.
  size_t                           groups;                                                          // Number of groups.
  size_t                           neighbours;                                                      // Number of neighbours.
  std::vector<GLint>               point;                                                           // Point on frame.
  size_t                           point_nodes;                                                     // Number of point nodes.
  float                            x_min   = -1.0f;                                                 // "x_min" spatial boundary [m].
  float                            x_max   = +1.0f;                                                 // "x_max" spatial boundary [m].
  float                            y_min   = -1.0f;                                                 // "y_min" spatial boundary [m].
  float                            y_max   = +1.0f;                                                 // "y_max" spatial boundary [m].
  float                            z_min   = -1.0f;                                                 // "z_min" spatial boundary [m].
  float                            z_max   = +1.0f;                                                 // "z_max" spatial boundary [m].
  float                            ds      = 1.0f;                                                  // Cell size [m].
  int                              ABCD    = 1;                                                     // Loop "ABCD".
  int                              EFGH    = 2;                                                     // Loop "EFGH".
  int                              ADHE    = 3;                                                     // Loop "ADHE".
  int                              BCGF    = 4;                                                     // Loop "BCGF".
  int                              ABFE    = 5;                                                     // Loop "ABFE".
  int                              DCGH    = 6;                                                     // Loop "DCGH".
  int                              AB_SIDE = 7;                                                     // Side "AB".
  int                              DA_SIDE = 8;                                                     // Side "DA".
  int                              VOLUME  = 1;                                                     // Entire volume.

  // SIMULATION VARIABLES:
  float                            m       = 10.0f;                                                 // Node mass [kg].
  float                            K       = 100.0f;                                                // Link elastic constant [kg/s^2].
  float                            B       = 1.0f;                                                  // Damping [kg*s*m].
  float                            R0      = 0.3f;                                                  // Nucleus radius [m].
  float                            dt_critical;                                                     // Critical time step [s].
  float                            dt_simulation;                                                   // Simulation time step [s].

  // BACKUP:
  std::vector<nu_float4_structure> initial_position;                                                // Backing up initial data...
  std::vector<nu_float4_structure> initial_position_int;                                            // Backing up initial data...
  std::vector<nu_float4_structure> initial_velocity;                                                // Backing up initial data...
  std::vector<nu_float4_structure> initial_velocity_int;                                            // Backing up initial data...
  std::vector<nu_float4_structure> initial_acceleration;                                            // Backing up initial data...

  /////////////////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////// DATA INITIALIZATION ///////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  // MESH:
  gravity->process (VOLUME, 3, NU_MSH_HEX_8);                                                       // Processing mesh...

  position->data  = gravity->node_coordinates;                                                      // Setting all node coordinates...
  neighbour->data = gravity->neighbour;                                                             // Setting neighbour indices...
  offset->data    = gravity->neighbour_offset;                                                      // Setting neighbour offsets...
  resting->data   = gravity->neighbour_length;                                                      // Setting resting distances...

  nodes           = gravity->node.size ();                                                          // Getting the number of nodes...
  elements        = gravity->element.size ();                                                       // Getting the number of elements...
  groups          = gravity->group.size ();                                                         // Getting the number of groups...
  neighbours      = gravity->neighbour.size ();                                                     // Getting the number of neighbours...

  std::cout << "nodes = " << nodes << std::endl;
  std::cout << "elements = " << elements << std::endl;
  std::cout << "groups = " << groups << std::endl;
  std::cout << "neighbours = " << neighbours << std::endl;
  std::cout << "offsets = " << gravity->neighbour_offset.size () << std::endl;
  std::cout << "lenghts = " << gravity->neighbour_length.size () << std::endl;
  std::cout << "links = " << gravity->neighbour_link.size () << std::endl;

  dt_critical     = sqrt (m/K);                                                                     // Critical time step [s].
  dt_simulation   = 0.02f*dt_critical;                                                              // Simulation time step [s].

  // SETTING NEUTRINO ARRAYS (parameters):
  friction->data.push_back (B);                                                                     // Setting friction...
  dt->data.push_back (dt_simulation);                                                               // Setting time step...
  radius->data.push_back (R0);                                                                      // Setting nucleus radius...

  // SETTING NEUTRINO ARRAYS ("nodes" depending):
  for(i = 0; i < nodes; i++)
  {
    position_int->data.push_back (position->data[i]);                                               // Setting intermediate position...
    velocity->data.push_back ({0.0f, 0.0f, 0.0f, 1.0f});                                            // Setting velocity...
    velocity_int->data.push_back ({0.0f, 0.0f, 0.0f, 1.0f});                                        // Setting intermediate velocity...
    acceleration->data.push_back ({0.0f, 0.0f, 0.0f, 1.0f});                                        // Setting acceleration...
    mass->data.push_back (m);                                                                       // Setting mass...
    freedom->data.push_back (1);                                                                    // Setting freedom flag...

    // Computing minimum element offset index:
    if(i == 0)
    {
      j_min = 0;                                                                                    // Setting minimum element offset index...
    }
    else
    {
      j_min = offset->data[i - 1];                                                                  // Setting minimum element offset index...
    }

    j_max = offset->data[i];                                                                        // Setting maximum element offset index...

    for(j = j_min; j < j_max; j++)
    {
      central->data.push_back (gravity->node[i]);                                                   // Building central node vector...
      stiffness->data.push_back (K);                                                                // Setting link stiffness...

      if(resting->data[j] > 0.21)
      {
        color->data.push_back ({0.0f, 0.0f, 0.0f, 0.0f});                                           // Setting color...
      }
      else
      {
        color->data.push_back ({0.0f, 1.0f, 0.0f, 1.0f});                                           // Setting color...
      }
    }
  }

  // SETTING MESH PHYSICAL CONSTRAINTS:
  gravity->process (ABCD, 2, NU_MSH_PNT);                                                           // Processing mesh...
  point                = gravity->node;                                                             // Getting nodes on border...
  point_nodes          = point.size ();                                                             // Getting the number of nodes on border...

  for(i = 0; i < point_nodes; i++)
  {
    freedom->data[point[i]] = 0;                                                                    // Resetting freedom flag...
  }

  gravity->process (EFGH, 2, NU_MSH_PNT);                                                           // Processing mesh...
  point                = gravity->node;                                                             // Getting nodes on border...
  point_nodes          = point.size ();                                                             // Getting the number of nodes on border...

  for(i = 0; i < point_nodes; i++)
  {
    freedom->data[point[i]] = 0;                                                                    // Resetting freedom flag...
  }

  gravity->process (ADHE, 2, NU_MSH_PNT);                                                           // Processing mesh...
  point                = gravity->node;                                                             // Getting nodes on border...
  point_nodes          = point.size ();                                                             // Getting the number of nodes on border...

  for(i = 0; i < point_nodes; i++)
  {
    freedom->data[point[i]] = 0;                                                                    // Resetting freedom flag...
  }

  gravity->process (BCGF, 2, NU_MSH_PNT);                                                           // Processing mesh...
  point                = gravity->node;                                                             // Getting nodes on border...
  point_nodes          = point.size ();                                                             // Getting the number of nodes on border...

  for(i = 0; i < point_nodes; i++)
  {
    freedom->data[point[i]] = 0;                                                                    // Resetting freedom flag...
  }

  gravity->process (ABFE, 2, NU_MSH_PNT);                                                           // Processing mesh...
  point                = gravity->node;                                                             // Getting nodes on border...
  point_nodes          = point.size ();                                                             // Getting the number of nodes on border...

  for(i = 0; i < point_nodes; i++)
  {
    freedom->data[point[i]] = 0;                                                                    // Resetting freedom flag...
  }

  gravity->process (DCGH, 2, NU_MSH_PNT);                                                           // Processing mesh...
  point                = gravity->node;                                                             // Getting nodes on border...
  point_nodes          = point.size ();                                                             // Getting the number of nodes on border...

  for(i = 0; i < point_nodes; i++)
  {
    freedom->data[point[i]] = 0;                                                                    // Resetting freedom flag...
  }

  // SETTING INITIAL DATA BACKUP:
  initial_position     = position->data;                                                            // Setting backup data...
  initial_position_int = position_int->data;                                                        // Setting backup data...
  initial_velocity     = velocity->data;                                                            // Setting backup data...
  initial_velocity_int = velocity_int->data;                                                        // Setting backup data...
  initial_acceleration = acceleration->data;                                                        // Setting backup data...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////// OPENCL KERNELS INITIALIZATION /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  K1->addsource (std::string (KERNEL_HOME) + std::string (UTILITIES));                              // Setting kernel source file...
  K1->addsource (std::string (KERNEL_HOME) + std::string (KERNEL_1));                               // Setting kernel source file...
  K1->build (nodes, 0, 0);                                                                          // Building kernel program...

  K2->addsource (std::string (KERNEL_HOME) + std::string (UTILITIES));                              // Setting kernel source file...
  K2->addsource (std::string (KERNEL_HOME) + std::string (KERNEL_2));                               // Setting kernel source file...
  K2->build (nodes, 0, 0);                                                                          // Building kernel program...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////// OPENGL SHADERS INITIALIZATION /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  S->addsource (std::string (SHADER_HOME) + std::string (SHADER_VERT), NU_VERTEX);                  // Setting shader source file...
  S->addsource (std::string (SHADER_HOME) + std::string (SHADER_GEOM), NU_GEOMETRY);                // Setting shader source file...
  S->addsource (std::string (SHADER_HOME) + std::string (SHADER_FRAG), NU_FRAGMENT);                // Setting shader source file...
  S->build (neighbours);                                                                            // Building shader program...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////// SETTING OPENCL KERNEL ARGUMENTS /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  cl->write ();

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////// APPLICATION LOOP ////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  while(!gl->closed ())                                                                             // Opening window...
  {
    cl->get_tic ();                                                                                 // Getting "tic" [us]...
    cl->acquire ();
    cl->execute (K1, NU_WAIT);                                                                      // Executing OpenCL kernel...
    cl->execute (K2, NU_WAIT);                                                                      // Executing OpenCL kernel...
    cl->release ();

    gl->clear ();                                                                                   // Clearing gl...
    gl->poll_events ();                                                                             // Polling gl events...
    gl->mouse_navigation (ms_orbit_rate, ms_pan_rate, ms_decaytime);
    gl->gamepad_navigation (gmp_orbit_rate, gmp_pan_rate, gmp_decaytime, gmp_deadzone);
    gl->plot (S);                                                                                   // Plotting shared arguments...

    gl->refresh ();                                                                                 // Refreshing gl...

    if(gl->button_CROSS)
    {
      gl->close ();                                                                                 // Closing gl...
    }

    if(gl->button_TRIANGLE)
    {
      position->data     = initial_position;                                                        // Restoring backup...
      position_int->data = initial_position_int;                                                    // Restoring backup...
      velocity->data     = initial_velocity;                                                        // Restoring backup...
      velocity_int->data = initial_velocity_int;                                                    // Restoring backup...
      acceleration->data = initial_acceleration;                                                    // Restoring backup...
      cl->write (1);                                                                                // Writing data...
      cl->write (2);                                                                                // Writing data...
      cl->write (3);                                                                                // Writing data...
      cl->write (4);                                                                                // Writing data...
      cl->write (5);                                                                                // Writing data...
    }

    cl->get_toc ();                                                                                 // Getting "toc" [us]...
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////// CLEANUP ////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  delete cl;                                                                                        // Deleting OpenCL context...
  delete color;                                                                                     // Deleting color data...
  delete position;                                                                                  // Deleting position data...
  delete position_int;                                                                              // Deleting intermediate position data...
  delete velocity;                                                                                  // Deleting velocity data...
  delete velocity_int;                                                                              // Deleting intermediate velocity data...
  delete acceleration;                                                                              // Deleting acceleration data...
  delete stiffness;                                                                                 // Deleting stiffness data...
  delete resting;                                                                                   // Deleting resting data...
  delete friction;                                                                                  // Deleting friction data...
  delete neighbour;                                                                                 // Deleting neighbours...
  delete offset;                                                                                    // Deleting offset...
  delete freedom;                                                                                   // Deleting freedom flag data...
  delete dt;                                                                                        // Deleting time step data...
  delete K1;                                                                                        // Deleting OpenCL kernel...
  delete K2;                                                                                        // Deleting OpenCL kernel...

  return 0;
}