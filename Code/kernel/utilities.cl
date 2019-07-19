#ifndef utilities_cl
#define utilities_cl

#include "client_datatypes.cl"

#define SAFEDIV(X, Y, EPSILON)    (X)/(Y + EPSILON)
#define RMIN                      0.4f                                          // Offset red channel for colormap
#define RMAX                      0.5f                                          // Maximum red channel for colormap
#define BMIN                      0.0f                                          // Offset blue channel for colormap
#define BMAX                      1.0f                                          // Maximum blue channel for colormap
#define SCALE                     1.5f                                          // Scale factor for plot

void link_displacements (
                          float4 P_R,                                           // Right neighbour position [m].
                          float4 P_U,                                           // Up neighbour position [m].
                          float4 P_L,                                           // Left neighbour position [m].
                          float4 P_D,                                           // Down neighbour position [m].
                          float4 P,                                             // Position [m].
                          float4 resting_R,                                     // Right neighbour resting position [m].
                          float4 resting_U,                                     // Up neighbour resting position [m].
                          float4 resting_L,                                     // Left neighbour resting position [m].
                          float4 resting_D,                                     // Down neighbour resting position [m].
                          float4 fr,                                            // Freedom flag [#].
                          float4* D_R,                                          // Right neighbour displacement [m].
                          float4* D_U,                                          // Up neighbour displacement [m].
                          float4* D_L,                                          // Left neighbour displacement [m].
                          float4* D_D                                           // Down neighbour displacement [m].
                        )
{
  ////////////////////////////////////////////////////////////////////////////////
  ////////////////// SYNERGIC MOLECULE: LINKED PARTICLE VECTOR ///////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      Ll_1 = Pl_1 - P;                                                  // 1st linked particle vector.
  float4      Ll_2 = Pl_2 - P;                                                  // 2nd linked particle vector.
  float4      Ll_3 = Pl_3 - P;                                                  // 3rd linked particle vector.
  float4      Ll_4 = Pl_4 - P;                                                  // 4th linked particle vector.

  ////////////////////////////////////////////////////////////////////////////////
  ///////////////////////// SYNERGIC MOLECULE: LINK LENGTH ///////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      ll_1 = length(Ll_1);                                              // 1st link length.
  float4      ll_2 = length(Ll_2);                                              // 2nd link length.
  float4      ll_3 = length(Ll_3);                                              // 3rd link length.
  float4      ll_4 = length(Ll_4);                                              // 4th link length.

  ////////////////////////////////////////////////////////////////////////////////
  ///////////////////////// SYNERGIC MOLECULE: LINK STRAIN ///////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      epsilon = fr - (float4)(1.0f, 1.0f, 1.0f, 1.0f);                  // Safety margin for division.
  float4      sl_1 = ll_1 - rl_1;                                               // 1st link strain.
  float4      sl_2 = ll_2 - rl_2;                                               // 2nd link strain.
  float4      sl_3 = ll_3 - rl_3;                                               // 3rd link strain.
  float4      sl_4 = ll_4 - rl_4;                                               // 4th link strain.

  ////////////////////////////////////////////////////////////////////////////////
  //////////////// SYNERGIC MOLECULE: LINKED PARTICLE DISPLACEMENT ///////////////
  ////////////////////////////////////////////////////////////////////////////////
  *Dl_1 = sl_1*SAFEDIV(Ll_1, ll_1, epsilon);                                                            // 1st linked particle displacement.
  *Dl_2 = sl_2*SAFEDIV(Ll_2, ll_2, epsilon);                                                            // 2nd linked particle displacement.
  *Dl_3 = sl_3*SAFEDIV(Ll_3, ll_3, epsilon);                                                          // 3rd linked particle displacement.
  *Dl_4 = sl_4*SAFEDIV(Ll_4, ll_4, epsilon);                                                             // 4th linked particle displacement.
}

float4 node_force (
                    float4  k_R,                                                // Right neighbour stiffness.
                    float4  k_U,                                                // Right neighbour stiffness.
                    float4  k_L,                                                // Right neighbour stiffness.
                    float4  k_D,                                                // Right neighbour stiffness.
                    float4  D_R,                                                // Right neighbour displacement [m].
                    float4  D_U,                                                // Up neighbour displacement [m].
                    float4  D_L,                                                // Left neighbour displacement [m].
                    float4  D_D,                                                // Down neighbour displacement [m].
                    float4  C,                                                  // Friction coefficient.
                    float4  V,                                                  // Velocity [m/s].
                    float4  m,                                                  // Mass [kg].
                    float4  g,                                                  // Gravity [m/s^2].
                    float4  fr                                                  // Freedom flag [#].
                  )
{
  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// SYNERGIC MOLECULE: ELASTIC FORCE //////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      Fe   = (kl_1*Dl_1 + kl_2*Dl_2 + kl_3*Dl_3 + kl_4*Dl_4);           // Elastic force applied to the particle.

  ////////////////////////////////////////////////////////////////////////////////
  //////////////////////// SYNERGIC MOLECULE: VISCOUS FORCE //////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      Fv   = -c*V;                                                      // Viscous force applied to the particle.

  ////////////////////////////////////////////////////////////////////////////////
  ///////////////////// SYNERGIC MOLECULE: GRAVITATIONAL FORCE ///////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      Fg   = m*G;                                                       // Gravitational force applied to the particle.

  ////////////////////////////////////////////////////////////////////////////////
  ///////////////////////// SYNERGIC MOLECULE: TOTAL FORCE ///////////////////////
  ////////////////////////////////////////////////////////////////////////////////
  float4      F    = fr*(Fe + Fv + Fg);                                         // Total force applied to the particle.

  return F;
}

void fix_projective_space (
                            float4* vector
                          )
{
  *vector *= (float4)(1.0f, 1.0f, 1.0f, 0.0f);                                  // Nullifying 4th projective component...

  *vector += (float4)(0.0f, 0.0f, 0.0f, 1.0f);                                  // Setting 4th projective component to "1.0f"...
}

// Assign color based on a custom colormap.
void assign_color(float4* color, float4* position)
{
  // Taking the component-wise absolute value of the position vector...
  float4 p = fabs(*position)*SCALE;

  // Extracting the z-component of the displacement...
  p *= (float4)(0.0f, 0.0f, 1.0f, 0.0f);

  // Setting color based on linear-interpolation colormap and adjusting alpha component...
  *color = (float4)(RMIN+(RMAX-RMIN)*p.z, 0.0f, BMIN+(BMAX-BMIN)*p.z, 1.0f);
}

#endif