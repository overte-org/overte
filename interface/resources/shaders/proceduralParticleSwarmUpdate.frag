uniform float lifespan = 1.0; // seconds
uniform float speed = 0.1; // m/s
uniform float speedSpread = 0.25;
uniform float mass = 1.0;

const float G = 6.67e-11;

// prop0: xyz: position, w: age
// prop1: xyz: velocity, w: prevUpdateTime
// prop2: xyz: prevVelocity

vec3 initPosition(const int particleID) {
  return 0.5 * (vec3(hifi_hash(particleID + iGlobalTime),
                     hifi_hash(particleID + iGlobalTime + 1.0),
                     hifi_hash(particleID + iGlobalTime + 2.0)) - 0.5);
}

mat3 rotationMatrix(vec3 axis, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}

vec3 initVelocity(const int particleID, const vec3 position) {
  const float particleSpeed = speed * ((1.0 - speedSpread) + speedSpread * hifi_hash(particleID + iGlobalTime + 3.0));
  vec3 r = normalize(iWorldPosition - position);
  float angle = 2.0 * 3.14159 * hifi_hash(particleID + iGlobalTime + 4.0);
  return particleSpeed * rotationMatrix(r, angle) * cross(r, vec3(0, 1, 0));
}

void updateParticleProps(const int particleID, inout ParticleUpdateProps particleProps) {
    // First draw
    if (particleProps.prop1.w < 0.00001) {
        particleProps.prop0.xyz = iWorldOrientation * (initPosition(particleID) * iWorldScale) + iWorldPosition;
        particleProps.prop0.w = -lifespan * hifi_hash(particleID + iGlobalTime + 3.0);
        particleProps.prop1.xyz = initVelocity(particleID, particleProps.prop0.xyz);
        particleProps.prop1.w = iGlobalTime;
        particleProps.prop2.xyz = particleProps.prop1.xyz;
        return;
    }

    // Particle expired
    if (particleProps.prop0.w >= lifespan) {
        particleProps.prop0.xyz = iWorldOrientation * (initPosition(particleID) * iWorldScale) + iWorldPosition;
        particleProps.prop0.w = 0.0;
        particleProps.prop1.xyz = initVelocity(particleID, particleProps.prop0.xyz);
        particleProps.prop1.w = iGlobalTime;
        particleProps.prop2.xyz = particleProps.prop1.xyz;
        return;
    }

    float dt = 0.01666666666;//max(0.0, iGlobalTime - particleProps.prop1.w);
    particleProps.prop2.xyz = particleProps.prop1.xyz;
    if (particleProps.prop0.w >= 0.0) {
      // gravitational acceleration
      vec3 r = iWorldPosition - particleProps.prop0.xyz;
      vec3 g = (G * mass / max(0.01, dot(r, r))) * r;

      // position
      particleProps.prop0.xyz += particleProps.prop1.xyz * dt + (0.5 * dt * dt) * g;
      // velocity
      particleProps.prop1.xyz += g * dt;
    }

    // age
    particleProps.prop0.w += dt;
    // prevUpdateTime
    particleProps.prop1.w = iGlobalTime;
}
