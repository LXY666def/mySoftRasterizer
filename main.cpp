#include"color.hpp"
#include"rasterizer.hpp"
#include"OBJ_Loader.hpp"
#include"My_loader.hpp"
#include"triangle.hpp"
#include"matrix.hpp"
#include"camera.hpp"
#include"VertexShader.hpp"
#include"FragmentShader.hpp"
#include"hdri.hpp"
#include"string"

// TODO list
// disp mapping ; bump mapping
// note that i change the code of z_interpolate, but haven't change the vert shader
// add origin to onb class

// warning !!!!!!!!!!!!!!!!!!!!!!!!
// 0.0 <= R,G,B <= 1.0 all the time until set_pixel turns [0,1) to [0,255]
// z-buffer uses v in camera space, and v_viewspace[i][2] = vert[i][2] = (v_viewspace[i][2] + near) * 255.999 / (far - near);
// getModelMatrix: translation is the final step!!!
// pay attention to homogeneous coords when mvp transformation is made

void PCSS();
void IBL();

int main() {
    CameraInfo camInfo;
    camInfo.type = SR_CAMERA_CREATE_INFO;
    camInfo.e = vec3(0, 0, 0, 1);
    camInfo.cam_coord = onb(vec3(0, 0, -1), vec3(0, 1, 0));
    camInfo.near = 0.1f;
    camInfo.far = 10.0f;
    camInfo.fov = 90;
    camInfo.aspect_ratio = 1.0;
    camInfo.width = 700;
    camInfo.w_h_ratio = 1.0;

    Camera cam(camInfo);
    matrix view = cam.getViewMatrix();
    matrix projection = cam.getProjectionMatrix();
    matrix model = Identity();
    model[2][3] = -2.5f;

    matrix mp = projection * model;
    vec3 point = mp * vec3(0, 0, 0, 1);
}

void IBL() {
    HDRI sky("./resource/christmas_photo_studio_01_1k.png", 10);

    ObjLoader load_dragon("./resource/dragon.obj");
    std::cout << "load obj done\n";

    matrix modelTransformation_dragon = getModelMatrix(vec3(30), 0, 90, 0, vec3(0));

    auto texture_gray = make_shared<Texture_solid>(color(100));

    std::vector<Triangle*> TriangleList_dragon = load_dragon.getTriangleList(texture_gray);
    std::cout << "list construction done\n";

    CameraInfo camInfo;
    camInfo.type = SR_CAMERA_CREATE_INFO;
    camInfo.e = vec3(0, 0, 30, 1);
    camInfo.cam_coord = onb(-camInfo.e, vec3(0, 1, 0));
    camInfo.near = 10;
    camInfo.far = 50;
    camInfo.fov = 90;
    camInfo.aspect_ratio = 1.0;
    camInfo.width = 700;
    camInfo.w_h_ratio = 1.0;

    Camera cam(camInfo);

    CameraInfo lightInfo;
    lightInfo.type = SR_DOT_LIGHT_CREATE_INFO;
    lightInfo.e = vec3(-20, 0, 20, 1);
    lightInfo.cam_coord = onb(-camInfo.e, vec3(0, 1, 0));
    lightInfo.intensity = vec3(5000);

    Camera light(lightInfo);

    VERT_SHADER_standard vertShader_bridge(cam, modelTransformation_dragon);
    FRAG_SHADER_phong fragShader(light);

    Rasterizer rl(camInfo);
    rl.setBackGround(cam, sky);
    rl.render(vertShader_bridge, fragShader, TriangleList_dragon, __mesh);
    std::cout << "\nrender done\n";

    my_image imagel(rl.width, rl.height);
    imagel.outputImage("./resource/aaaoutput.ppm", rl.FrameBuffer().data());
    std::cout << "output image done\n";
}

void PCSS() {
    // loader
    ObjLoader load_bridge("./resource/bridge.obj");
    ObjLoader load_plane("./resource/plane.obj");
    std::cout << "load obj done\n";

    // model transform
    vec3 origin(0, 0, 0);
    vec3 bridge_pos = vec3(0, 8, 0) - origin;
    vec3 plane_pos = vec3(0, 0, 0) - origin;
    matrix modelTransformation_bridge = getModelMatrix(vec3(4), 0, 45, 0, bridge_pos);
    matrix modelTransformation_plane = getModelMatrix(vec3(25), 0, 45, 0, plane_pos);

    // texture
    auto texture_gray = make_shared<Texture_solid>(color(100));
    auto texture_red = make_shared<Texture_solid>(color(100,0,0));

    // triangle list
    std::vector<Triangle*> TriangleList_bridge = load_bridge.getTriangleList(texture_gray);
    std::vector<Triangle*> TriangleList_plane = load_plane.getTriangleList(texture_gray);
    std::cout << "list construction done\n";

    // Light
    CameraInfo lightInfo;
    lightInfo.type = SR_DOT_LIGHT_CREATE_INFO;
    lightInfo.e = vec3(-20, 30, -20, 1);
    lightInfo.cam_coord = onb(origin - lightInfo.e, vec3(0, 1, 0));
    lightInfo.near = 10;
    lightInfo.far = 70;
    lightInfo.fov = 90;
    lightInfo.aspect_ratio = 1.0;
    lightInfo.width = 512;
    lightInfo.w_h_ratio = 1.0;
    lightInfo.intensity = color(WHITE * 4000);
    lightInfo.radius = 15;

    Light light(lightInfo);

    // shader
    VERT_SHADER_standard vertShader_bridge(light, modelTransformation_bridge);
    VERT_SHADER_standard vertShader_plane(light, modelTransformation_plane);
    FRAG_SHADER_texture_image fragShader;

    Rasterizer rl(lightInfo);
    rl.render(vertShader_bridge, fragShader, TriangleList_bridge, __mesh);
    rl.render(vertShader_plane, fragShader, TriangleList_plane, __mesh);
    std::cout << "\nrender done\n";

    my_image imagel(rl.width, rl.height);
    imagel.outputImage("./resource/aaadepth.ppm", rl.DepthBuffer().data());
    light.claimDepthImage(rl.getDepthBuffer());
    std::cout << "output image done\n";

    // Camera
    CameraInfo camInfo;
    camInfo.type = SR_CAMERA_CREATE_INFO;
    camInfo.e = vec3(0, 30, 40, 1);
    camInfo.cam_coord = onb(origin - camInfo.e, vec3(0, 1, 0));
    camInfo.near = 10;
    camInfo.far = 70;
    camInfo.fov = 90;
    camInfo.aspect_ratio = 1.0;
    camInfo.width = 700;
    camInfo.w_h_ratio = 1.0;

    Camera cam(camInfo);
    light.toCamSpace(cam);
    
    // shader
    VERT_SHADER_standard cam_vertShader_bridge(cam, modelTransformation_bridge);
    VERT_SHADER_standard cam_vertShader_plane(cam, modelTransformation_plane);
    FRAG_SHADER_phong cam_fragShader(light);

    Rasterizer r(camInfo);
    r.render(cam_vertShader_bridge, cam_fragShader, TriangleList_bridge, __mesh);
    r.render(cam_vertShader_plane, cam_fragShader, TriangleList_plane, __mesh);
    std::cout << "\nrender done\n";

    my_image image(r.width, r.height);
    image.outputImage("./resource/aaaoutput.ppm", r.FrameBuffer().data());
    std::cout << "output image done\n";
}