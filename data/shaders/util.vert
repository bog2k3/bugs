mat4 RotationMatrix(float Angle)
{
	return mat4( cos( Angle ), -sin( Angle ), 0.0, 0.0,
			     sin( Angle ),  cos( Angle ), 0.0, 0.0,
			              0.0,           0.0, 1.0, 0.0,
				          0.0,           0.0, 0.0, 1.0 );
}
