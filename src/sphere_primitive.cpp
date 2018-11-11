#include <sphere_primitive.h>

#include <light_ray.h>
#include <intersection_point.h>

manta::SpherePrimitive::SpherePrimitive() {

}

manta::SpherePrimitive::~SpherePrimitive() {

}

void manta::SpherePrimitive::detectIntersection(const LightRay *ray, IntersectionPoint *p) {
	math::Vector d_pos = math::sub(ray->getSource(), m_position);
	math::Vector d_dot_dir = math::dot(d_pos, ray->getDirection());
	math::Vector mag2 = math::magnitudeSquared3(d_pos);

	math::Vector radius2 = math::loadScalar(m_radius * m_radius);
	math::Vector det = math::sub(math::mul(d_dot_dir, d_dot_dir), math::sub(mag2, radius2));

	if (math::getScalar(det) < 0.0f) {
		p->m_intersection = false;
	}
	else {
		det = math::sqrt(det);
		math::Vector t1 = math::sub(det, d_dot_dir);
		math::Vector t2 = math::sub(math::negate(det), d_dot_dir);

		float t1_s = math::getScalar(t1);
		float t2_s = math::getScalar(t2);

		p->m_intersection = t1_s > 0.0f || t2_s > 0.0f;

		if (p->m_intersection) {
			math::Vector t;
			float depth;
			if (t2_s < t1_s && t2_s >= 0.0f) {
				t = t2;
				p->m_depth = t2_s;
			} 
			else {
				t = t1;
				p->m_depth = t1_s;
			}

			p->m_position = math::add(ray->getSource(), math::mul(ray->getDirection(), t));

			// Calculate the normal
			math::Vector normal = math::sub(p->m_position, m_position);
			normal = math::normalize(normal);

			p->m_normal = normal;

		}
	}
}

