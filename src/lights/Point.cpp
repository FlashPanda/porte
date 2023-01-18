// lights/point.cpp*
#include <lights/Point.h>
#include <core/Scene.h>
#include <core/Sampling.h>

namespace porte {

Spectrum PointLight::Sample_Li(const Interaction &ref, const Point2f &u,
                               Vector3f *wi, Float *pdf,
                               VisibilityTester *vis) const {
    //ProfilePhase _(Prof::LightSample);
    *wi = Normalize(pLight - ref.p);
    *pdf = 1.f;
    *vis =
        VisibilityTester(ref, Interaction(pLight, ref.time/*, mediumInterface*/));
    return I / DistanceSquared(pLight, ref.p);
}

Spectrum PointLight::Power() const { return 4 * Pi * I; }

Float PointLight::Pdf_Li(const Interaction &, const Vector3f &) const {
    return 0;
}

Spectrum PointLight::Sample_Le(const Point2f &u1, const Point2f &u2, Float time,
                               Ray *ray, Normal3f *nLight, Float *pdfPos,
                               Float *pdfDir) const {
    //ProfilePhase _(Prof::LightSample);
    *ray = Ray(pLight, UniformSampleSphere(u1), Infinity, time/*,
               mediumInterface.inside*/);
    *nLight = (Normal3f)ray->d;
    *pdfPos = 1;
    *pdfDir = UniformSpherePdf();
    return I;
}

void PointLight::Pdf_Le(const Ray &, const Normal3f &, Float *pdfPos,
                        Float *pdfDir) const {
    //ProfilePhase _(Prof::LightPdf);
    *pdfPos = 0;
    *pdfDir = UniformSpherePdf();
}

std::shared_ptr<PointLight> CreatePointLight(const Transform &light2world, const Spectrum I) {
	//Spectrum I = paramSet.FindOneSpectrum("I", Spectrum(1.0));
	//Spectrum sc = paramSet.FindOneSpectrum("scale", Spectrum(1.0));
	//Point3f P = paramSet.FindOnePoint3f("from", Point3f(0, 0, 0));
 //   Transform l2w = Translate(Vector3f(P.x, P.y, P.z)) * light2world;
    return std::make_shared<PointLight>(light2world, I);
}

}  // namespace porte
