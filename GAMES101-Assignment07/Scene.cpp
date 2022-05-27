//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0.0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }

    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0.0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(const Ray &ray,const std::vector<Object*> &objects,float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }

    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here;

    //p is the intersection between object surface and ray_input;
    //x is the intersection between light and object surface;

    //首先将这条光线与场景做交，返回得到最近的交点p的属性;
    Intersection P = intersect(ray);
    //如果射线与场景中的物体不相交,返回黑色（不可见）;
    if (!P.happened) {
        return Vector3f();
    }
    //如果交点p的材质是光源，说明与光源直接相交，获取光源材质;
    if (P.m->hasEmission()) {
        return P.m->getEmission();
    }

    //光源的直接影响;
    Vector3f L_dir;

    //从p点向光源采样点x发射射线;
    Intersection light_x;//光源上的采样点x;
    float light_pdf=0.0f;//继续弹射的概率pdf = 0;
    //对光源进行采样（积分）;
    sampleLight(light_x, light_pdf);

    // Get x, ws, NN, emit from intersection;
    Vector3f p = P.coords;//光线与场景物体交点p的坐标;
    Vector3f x = light_x.coords;//光源上采样点x的坐标;
    Vector3f px_dir = (x - p).normalized();//P，x的方向向量;
    float px_distance = (x - p).norm();//P，x之间的距离;
    Vector3f N = P.normal.normalized();//交点P的法向量;
    Vector3f NN = light_x.normal.normalized();//光源采样点x的法向量;
    //Vector3f emit = light_x.emit;//?

    //shot a ray from p to light_x;
    Ray px_ray(p,px_dir);
    Intersection x_p = intersect(px_ray);//求这条射线与场景的交点;
    //如果光线px在p到x之间无阻隔;
    if (x_p.distance - px_distance > -EPSILON) {
        //返回P;
        L_dir = light_x.emit * P.m->eval(ray.direction, px_ray.direction, N)
            * dotProduct(px_ray.direction, N)
            * dotProduct(-px_ray.direction, NN)
            / std::pow(px_distance, 2) 
            / light_pdf;
    }

    // Test Russian Roulette with probability RussianRoulette;
    if (get_random_float() > RussianRoulette) {
        return L_dir;
    }

    //其他物体的间接光照影响;
    Vector3f L_indir = 0.0;
    //采样一个光线输入的方向;
    Vector3f wi_dir = P.m->sample(ray.direction, N).normalized();
    //从p点向外方向输出射出光线;
    Ray wi_ray(P.coords, wi_dir);
    //折射光线与场景物体有交点;
    Intersection wi_p = intersect(wi_ray);//求出射光线与场景中物体的交点;
    //如果折射光线与场景物体有交点，（递归）返回该交点对p的影响;
    if (wi_p.happened && (!wi_p.m->hasEmission())) {
        L_indir = castRay(wi_ray, depth + 1)
            * P.m->eval(ray.direction, wi_ray.direction, N)
            * dotProduct(wi_ray.direction, N)
            / P.m->pdf(ray.direction, wi_ray.direction, N)
            / RussianRoulette;
    }
    return L_dir + L_indir;

}