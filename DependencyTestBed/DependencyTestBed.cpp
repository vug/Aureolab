#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <entt/entt.hpp>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>

#include <memory>
#include <sstream>

std::shared_ptr<spdlog::logger> CreateLogger() {
	std::vector<spdlog::sink_ptr> sinks = {
		std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
	};
	sinks[0]->set_pattern("%^[%T.%e] %n: %v%$");
	auto logger = std::make_shared<spdlog::logger>("DEPTESTBED", begin(sinks), end(sinks));
	logger->set_level(spdlog::level::trace);
	logger->flush_on(spdlog::level::trace);
	return logger;
}

auto LOGGER = CreateLogger();

static void error_callback(int error, const char* description)
{
    LOGGER->error("Error [{}]: {}", error, description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct Position {
    float x;
    float y;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(x), CEREAL_NVP(y)); // CEREAL_NVP let's the serialization name to be the same as variable name. otherwise name becomes valueN.
        //ar(CEREAL_NVP(x));
        //ar(CEREAL_NVP(y));
    }
};

struct Tag {
    std::string tag;

    template <class Archive>
    void serialize(Archive& ar) {
        ar(CEREAL_NVP(tag));
    }
};

// this crashes :-/
//void prologue(cereal::JSONOutputArchive& ar, const Position& p) {
//    LOGGER->debug("Hi from prologue");
//}

int main(int argc, char* argv[]) {
    LOGGER->info("Hi from Dependency Test Bed!");
    entt::registry registry;
    auto ent1 = registry.create();
    auto ent2 = registry.create();
    registry.emplace<Position>(ent1, 1.0f, 2.0f);
    registry.emplace<Tag>(ent1, "object-1");
    registry.emplace<Position>(ent2, 3.0f, 4.0f);
    registry.emplace<Tag>(ent2, "object-2");
    auto view = registry.view<Tag, Position>();

    for (auto [entity, tag, pos] : view.each()) {
        LOGGER->info("Entity {} [{}], Position: ({}, {})", tag.tag, entity, pos.x, pos.y);
    //    std::stringstream ss;
    //    {
    //        cereal::JSONOutputArchive oarchive(ss);
    //        oarchive(CEREAL_NVP(pos));
    //        LOGGER->info("Comp Serialized: {}", ss.str());
    //    }

    //    {
    //        cereal::JSONInputArchive iarchive(ss);
    //        Position posDeser;
    //        iarchive(posDeser);
    //        LOGGER->info("Comp Deserialized: ({}, {})", posDeser.x, posDeser.y);
    //    }
    }

    std::stringstream ss;
    {
        cereal::JSONOutputArchive oarchive(ss);
        auto snapshot = entt::snapshot{ registry };
        snapshot.entities(oarchive).component<Tag, Position>(oarchive);
    }
    LOGGER->info("Reg Serialized: {}", ss.str());

    entt::registry registry2;
    cereal::JSONInputArchive iarchive{ ss };
    entt::snapshot_loader{ registry2 }.entities(iarchive).component<Tag, Position>(iarchive);
    auto view2 = registry2.view<Tag, Position>();
    for (auto [entity, tag, pos] : view2.each()) {
        LOGGER->info("Entity {} [{}], Position: ({}, {})", tag.tag, entity, pos.x, pos.y);
    }

    static const struct
    {
        float x, y;
        float r, g, b;
    } vertices[] =
    {
        { -0.6f, -0.4f, 1.f, 0.f, 0.f },
        {  0.6f, -0.4f, 0.f, 1.f, 0.f },
        {   0.f,  0.6f, 0.f, 0.f, 1.f },
        { -0.3f,  0.8f, 1.f, 0.f, 1.f },
    };
    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    static const char* vertex_shader_text =
        "#version 460 core\n"
        "uniform mat4 MVP;\n"
        "layout (location = 0) in vec2 vPos;\n"
        "layout (location = 1) in vec3 vCol;\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
        "    color = vCol;\n"
        "}\n";

    static const char* fragment_shader_text =
        "#version 460 core\n"
        "varying vec3 color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(color, 1.0);\n"
        "}\n";

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        LOGGER->critical("Could not initialize GLFW. Exiting...");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Dependency Test Bed", NULL, NULL);
    if (!window) {
        glfwTerminate();
        LOGGER->critical("Could not initialize GLFW window. Exiting...");
        return 1;
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(0);  // VSync {0: disabled, 1: enabled}

    // ImGui Init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    // platform/renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // NOTE: OpenGL error checks have been omitted for brevity

    GLuint vertex_shader, fragment_shader, program;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);


    GLint mvp_location, vpos_location, vcol_location;

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");


    GLuint vbo, vao, ebo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)0);
    glEnableVertexAttribArray(vpos_location);

    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(vertices[0]), (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(vcol_location);

    LOGGER->info("OpenGL Info:");
    LOGGER->info("Renderer: {}", glGetString(GL_RENDERER));
    LOGGER->info("Vendor: {}", glGetString(GL_VENDOR));
    LOGGER->info("Version: {}", glGetString(GL_VERSION));


    while (!glfwWindowShouldClose(window)) {
        // ImGUI Begin
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Begin("ImGUI Window");
        ImGui::Text("I'm a text!");
        ImGui::End();

        float ratio;
        int width, height;
        glm::mat4 mvp(1.0f);
        mvp = glm::rotate(mvp, (float)glfwGetTime(), glm::vec3(0.0f, 0.1f, 0.1f));

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
        glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(unsigned int), GL_UNSIGNED_INT, 0);

        // ImGUI End
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // Update and Render additional Platform Windows (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context); //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ImGUI Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // optional
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
	return 0;
}