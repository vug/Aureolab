#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <GLFW/glfw3.h>

#include <memory>

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

int main(int argc, char* argv[]) {
	auto logger = CreateLogger();
	logger->info("Hi from Dependency Test Bed!");

    GLFWwindow* window;
    glfwInit();
    window = glfwCreateWindow(1024, 768, "Dependency Test Bed", NULL, NULL);
    glfwMakeContextCurrent(window);
    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
        glRotatef((float)glfwGetTime() * 50.f, 0.f, 0.f, 1.f);

        glBegin(GL_TRIANGLES);
        glColor3f(1.f, 0.f, 0.f);
        glVertex3f(-0.6f, -0.4f, 0.f);
        glColor3f(0.f, 1.f, 0.f);
        glVertex3f(0.6f, -0.4f, 0.f);
        glColor3f(0.f, 0.f, 1.f);
        glVertex3f(0.f, 0.6f, 0.f);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
	return 0;
}