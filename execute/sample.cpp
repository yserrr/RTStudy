#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <fstream>
#include <cmath>
#include <glm/glm.hpp>
#include <ctime>
#include <algorithm>

const float INF = std::numeric_limits<float>::infinity() ;
typedef std::pair   <,float> Sphere ;
typedef std::vector <Sphere>  World;
typedef glm::vec3  Color;
typedef glm::vec3  Point;

Color background(Ray ray){
    glm::vec3  sun_dir = unitVector(glm::vec3 (0, 1 ,1)); // 태양 방향
    float sun_intensity = std::pow(std::max(0.0f, dot(unitVector(ray.direction()), sun_dir)), 200);
    Color sun_color = Color(1.0, 0.95, 0.8) * sun_intensity * 100000; // 강한 노란빛
    Color sky = (1-ray.dir.y())*Color(1,1,1) + ray.dir.y()*Color(0.5, 0.7, 1.0);
    return clamp(sky + sun_color, 0.0, 1.0);
}


Sphere hitSphere(Ray ray, World world){
    Sphere s ;
    s.first = Point(0,0,0);
    s.second = INF ;
    for(auto i: world){
        glm::vec3  oc = i.first-ray.origin();

        float a = dot(ray.direction(), ray.direction());
        float b = -2.0 * dot(oc, ray.direction());
        float c = dot(oc, oc) - i.second * i.second;
        float discriminant = b * b - 4 * a * c;
        if (discriminant <0) continue;
        float temp = (-b -std::sqrt(discriminant)) / (2 * a);
        if(temp<0) continue ;
        if( temp< s.second  ){
            s.second = temp;
            s.first  = i.first;
        }
    }
    return s ;
}

Color trace(Ray ray, World world , int depth){

    Color color(0,0,0);
    if (depth==0) return color;

    Sphere s = hitSphere(ray, world);
    if (s.second==INF ) return background(ray);
    auto orig = ray.at(s.second);
    auto N = orig-s.first;
    N= unitVector(N);
    N= N+glm::vec3 (randomfloat(-1,1),randomfloat(-1,1),randomfloat(-1,1));
    N= unitVector(N );
    if(dot(N,ray.dir)>0) N*= -1;
    Ray scatter(orig, N);
    return 0.5* trace(scatter, world,depth-1);
}

Color traceMetal(Ray ray, World world, int depth, float fuzz){
    if(depth==0) return Color(0,0,0);
    Sphere s = hitSphere(ray, world);
    if (s.second==INF ) return background(ray);
    auto orig = ray.at(s.second);
    auto N = unitVector(orig - s.first);
    glm::vec3  reflected = reflect(unitVector(ray.direction()), N);
    reflected = unitVector(reflected + fuzz * glm::vec3 (randomfloat(-1,1),randomfloat(-1,1),randomfloat(-1,1)));
    Ray scatter(orig, reflected);
    if(dot(scatter.direction(), N) > 0)
        return 0.8* traceMetal(scatter, world, depth-1, fuzz);
    else
        return Color(0,0,0);
}
int main(){
    // 파일명에 날짜와 시간을 붙여서 계속 새로운 파일이 생성되도록 함
    time_t now = time(nullptr);
    char time_buffer[100];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y%m%d_%H%M%S", std::localtime(&now));
    std::string filename = std::string("output_") + time_buffer + ".ppm";
    std::fstream out;
    out.open(filename, std::ios::out);
    if(!out.is_open()) err();

    //image ->not ratio but physical value

    auto center = glm::vec3  (0,0,0);

    //PHYSICAL VALUE
    int height = 256;
    int width  = 256;
    float focalLength =100.0f;
    int samplingNumber =16;

    //MATH MODEL
//    float viewportWidth = 2;
//    float viewportHeight= 2;
//    float focalLength = 1.0;//focalLength와 viewport -> ratio
//    glm::vec3  x = glm::vec3 (viewportWidth, 0, 0 );
//    glm::vec3  y = glm::vec3 (0,-viewportHeight, 0);
//    glm::vec3  depth = glm::vec3 (0,0,-focalLength);
//    glm::vec3  start = center + depth - x/2- y/2;

    glm::vec3  start(-width/2 +0.5,height/2+0.5, -focalLength);

    World world;
    world.push_back(std::make_pair(glm::vec3 (0,        0,-1) , 0.5));
    world.push_back(std::make_pair(glm::vec3 (0,   -100.5,-1) , 100));

    (out) << "P3\n" << width << ' ' << height << "\n255\n"; //PPM format header

    for(int i=0; i< height; i++ ){
        for(int j=0; j< width; j++){
            Color color(0,0,0);
            //float u = float(j)/ (width-1);
            //float v = float(i)/ (height-1) ;
            glm::vec3  direction = start + glm::vec3 (j, -i, 0);
            //auto direction = start+ u*x+v*y;
            for(int k=0; k<samplingNumber; k++ ){
                glm::vec3  dir = direction  +0.5 * glm::vec3 (randomfloat(-1,1),randomfloat(-1,1),randomfloat(-1,1));
                dir = unitVector(dir);
                Ray ray(center, dir);
                //color+= traceMetal(ray, world,10,0.2);
                color+= trace(ray,world,5);
            }
            color /= samplingNumber;
            // 출력 직전
            out << int(255 * clamp(color.x(), 0.0, 1.0)) << ' '
                << int(255 * clamp(color.y(), 0.0, 1.0)) << ' '
                << int(255 * clamp(color.z(), 0.0, 1.0)) << std::endl;
        }
    }
}
