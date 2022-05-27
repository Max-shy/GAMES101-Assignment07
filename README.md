# GAMES101-Assignment07
Assignment 7 needs to complete the algorithm of Path Tracing based on Assignment 6.

## Assignment 07

Assignment 7 needs to complete the algorithm of Path Tracing based on Assignment 6. Although I learned enough about the flow of the Path Tracing algorithm, it still took me a long time. Mostly because I didn't understand how to use auxiliary functions. 

As in the assignment 6, enter the castRay() function from main(). This time we need to complete the whole castRay() function according to the pseudocode. And this function is divided into two parts.

Before proceeding to these two parts, I first need to determine whether the light intersects with the objects in the scene and whether it intersects directly and at the light source.

```CPP
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
```



The first part is to complete the influence of the light source on the intersection of the current ray and the scene.

![image-20220527142705259](E:\CG\Games\GAMES101\study report\Week2\Study report for week 2.assets\image-20220527142705259.png)

Here, it is necessary to sample the light source and judge whether the light source and the intersection point have occlusion.

```cpp
	//光源对交点P的直接影响;
    Vector3f L_dir;

    //从p点向光源采样点x发射射线;
    Intersection light_x;//光源上的采样点x;
    float light_pdf=0.0f;//继续弹射的概率pdf = 0;
    //对光源进行采样（积分）;
    sampleLight(light_x, light_pdf);

    // Get P,light coordinate、normal and distance from intersection;
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
    	//L_dir = emit*eval(wo,ws,N)*dot(ws,N)*dot(ws,NN)/|x-p|^2/pdf_light
        L_dir = light_x.emit * P.m->eval(ray.direction, px_ray.direction, N)
            * dotProduct(px_ray.direction, N)
            * dotProduct(-px_ray.direction, NN)
            / std::pow(px_distance, 2) 
            / light_pdf;
    }
```

The second part involves recursively calculating the effect of indirect light on the current intersection P.

![image-20220527145342978](E:\CG\Games\GAMES101\study report\Week2\Study report for week 2.assets\image-20220527145342978.png)

```CPP
	//其他物体对P的间接光照影响;
    Vector3f L_indir = 0.0;
    //采样一个光线输入的方向;
    Vector3f wi_dir = P.m->sample(ray.direction, N).normalized();
    //从p点向外方向输出射出采样的光线;
    Ray wi_ray(P.coords, wi_dir);
    //采样光线与场景物体有交点;
    Intersection wi_p = intersect(wi_ray);//求出射光线与场景中物体的交点;
    //如果折射光线与场景物体有交点，（递归）返回该交点对p的影响;
    if (wi_p.happened && (!wi_p.m->hasEmission())) {
        L_indir = castRay(wi_ray, depth + 1)
            * wi_p.m->eval(ray.direction, wi_ray.direction, N)
            * dotProduct(wi_ray.direction, N)
            / P.m->pdf(ray.direction, wi_ray.direction, N)
            / RussianRoulette;
    }
    return L_dir + L_indir;
```

Let's take a look at the render result.

<img src="E:\CG\Games\GAMES101\study report\Week2\pic\binary3_1.jpg" align=mid width="70%" height="70%"/>

Something serious has gone wrong. I need to recheck the algorithm code. It turned out to be a precision problem. The EPSILON value was set too low. Increase EPSILON.

```CPP
const float EPSILON = 0.00016;
```

Look at the render result again.


<center>
	<img src="E:\CG\Games\GAMES101\study report\Week2\pic\binary3_2.jpg" width="40%" />
	&emsp;&emsp;&emsp;&emsp;
	<img src="E:\CG\Games\GAMES101\study report\Week2\pic\image-20220527213946646.png" width="40%" />
	<br/>
	<font color="AAAAAA">My Result</font>
	&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
	&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
	&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
	<font color="AAAAAA">correct Result</font>
</center>


It seems to be brighter than the standard answer.

It turns out that I forgot to test the probability of Russian roulette against Russian roulette to determine if the current Ray continues to refract before calculating indirect lighting.

```CPP
    // Test Russian Roulette with probability RussianRoulette;
    if (get_random_float() > RussianRoulette) {
        return L_dir;
    }   
```

<center>
	<img src="E:\CG\Games\GAMES101\study report\Week2\pic\binary3_3.jpg" width="40%" />
	&emsp;&emsp;&emsp;&emsp;
	<img src="E:\CG\Games\GAMES101\study report\Week2\pic\image-20220527213946646.png" width="40%" />
	<br/>
	<font color="AAAAAA">My Result</font>
	&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
	&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
	&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;
	<font color="AAAAAA">correct Result</font>
</center>

Now the result seems to be correct.
