#include <jni.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <cstdlib>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

#define LOG_TAG "ArkModMenu"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

const char *GAME_PACKAGE = "com.studiowildcard.arkuse";
uintptr_t libBase = getLibBase("libUE4.so");

bool unlimitedHealth = false;
bool highJump = false;
bool godMode = false;
bool instantTame = false;
bool infiniteStamina = false;
bool rapidFire = false;
int GWorld = 0;
int GName = 0;
int UObjects = 0;

struct Memory {
    uint64_t base;
    uint64_t GWorld;
    uint64_t GNames;
    uint64_t UObject;
} Memory;

bool isGameRunning() {
    DIR *procDir = opendir("/proc");
    if (!procDir) return false;

    struct dirent *entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            char cmdPath[256];
            snprintf(cmdPath, sizeof(cmdPath), "/proc/%s/cmdline", entry->d_name);

            FILE *cmdFile = fopen(cmdPath, "r");
            if (cmdFile) {
                char cmdline[256];
                fgets(cmdline, sizeof(cmdline), cmdFile);
                fclose(cmdFile);

                if (strstr(cmdline, GAME_PACKAGE)) {
                    closedir(procDir);
                    return true;
                }
            }
        }
    }
    closedir(procDir);
    return false;
}

void launchGame() {
    LOGD("Launching game: %s", GAME_PACKAGE);

    char launchCommand[256];
    snprintf(launchCommand, sizeof(launchCommand), "am start -n %s/.MainActivity", GAME_PACKAGE);

    system(launchCommand);
}

// Get the base address of libUE4.so
uintptr_t getLibBase(const char *libName) {
    FILE *fp = fopen("/proc/self/maps", "rt");
    if (!fp) return 0;

    uintptr_t baseAddr = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, libName)) {
            sscanf(line, "%lx", &baseAddr);
            break;
        }
    }
    fclose(fp);
    return baseAddr;
}

void enableUnlimitedHealth() {
    uintptr_t libBase = getLibBase("libUE4.so");
    if (libBase) {
        uintptr_t healthAddr = libBase + 0x3A76F40;
        *(float *)healthAddr = 999999.0f;
        LOGD("Unlimited Health enabled!");
    } else {
        LOGD("libUE4.so not found!");
    }
}

void enableHighJump() {
    uintptr_t libBase = getLibBase("libUE4.so");
    if (libBase) {
        uintptr_t jumpAddr = libBase + 0x3B12E80;
        *(float *)jumpAddr = 1000.0f;
        LOGD("High Jump enabled!");
    } else {
        LOGD("libUE4.so not found!");
    }
}

void enableGodMode() {
    uintptr_t libBase = getLibBase("libUE4.so");
    if (libBase) {
        uintptr_t godModeAddr = libBase + 0x3A89CC0;
        *(bool *)godModeAddr = true;
        LOGD("God Mode enabled!");
    }
}

void enableInstantTame() {
    uintptr_t libBase = getLibBase("libUE4.so");
    if (libBase) {
        uintptr_t tameAddr = libBase + 0x3AC4580;
        *(float *)tameAddr = 99999.0f;
        LOGD("Instant Tame enabled!");
    }
}

void enableInfiniteStamina() {
    uintptr_t libBase = getLibBase("libUE4.so");
    if (libBase) {
        uintptr_t staminaAddr = libBase + 0x3A98D40;
        *(float *)staminaAddr = 999999.0f;
        LOGD("Infinite Stamina enabled!");
    }
}

void enableRapidFire() {
    uintptr_t libBase = getLibBase("libUE4.so");
    if (libBase) {
        uintptr_t fireRateAddr = libBase + 0x3B45A20;
        *(float *)fireRateAddr = 0.001f;
        LOGD("Rapid Fire enabled!");
    }
}

void InitMemoryAddresses() {
    Memory.base = getLibBase("libUE4.so");
    if (!Memory.base) {
        LOGD("Failed to get libUE4.so base address!");
        return;
    }

    // ARK UE4 specific offsets
    Memory.GWorld = Memory.base + 0x7C03228;  // GWorld offset
    Memory.GNames = Memory.base + 0x7B7C240;  // GNames offset
    Memory.UObject = Memory.base + 0x7BA1A40; // UObject offset
    
    LOGD("Memory Base: 0x%lx", Memory.base);
    LOGD("GWorld: 0x%lx", Memory.GWorld);
    LOGD("GNames: 0x%lx", Memory.GNames);
    LOGD("UObject: 0x%lx", Memory.UObject);
}

uint64_t ReadGWorld() {
    return *(uint64_t*)Memory.GWorld;
}

uint64_t GetGNamesByID(int32_t id) {
    uint64_t GNames = *(uint64_t*)Memory.GNames;
    if (!GNames) return 0;
    
    uint64_t chunk = *(uint64_t*)(GNames + ((id / 0x4000) * 8));
    if (!chunk) return 0;
    
    return *(uint64_t*)(chunk + ((id % 0x4000) * 8));
}

void initOpenGL() {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    EGLConfig config;
    EGLint numConfigs;
    EGLint attributes[] = {EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE};
    eglChooseConfig(display, attributes, &config, 1, &numConfigs);

    EGLSurface surface = eglCreatePbufferSurface(display, config, nullptr);

    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    eglMakeCurrent(display, surface, surface, context);
}

void renderMenu() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Ark Mod Menu");
    
    uint64_t currentGWorld = ReadGWorld();
    ImGui::Text("Base: 0x%lX", Memory.base);
    ImGui::Text("GWorld: 0x%lX", currentGWorld);
    ImGui::Text("GNames: 0x%lX", *(uint64_t*)Memory.GNames);
    ImGui::Text("UObject: 0x%lX", *(uint64_t*)Memory.UObject);
    
    // Example of getting a name
    if (ImGui::Button("Test GName")) {
        uint64_t testName = GetGNamesByID(1);
        if (testName) {
            LOGD("Name ID 1: %s", (char*)(testName + 0x10));
        }
    }

    if (ImGui::Checkbox("Unlimited Health", &unlimitedHealth)) {
        if (unlimitedHealth) enableUnlimitedHealth();
    }
    if (ImGui::Checkbox("High Jump", &highJump)) {
        if (highJump)
        {
            ImGui::SliderFloat("Jump Height", &jumpHeight, 1.0f, 1000.0f);
        }
    }
    if (ImGui::Checkbox("God Mode", &godMode)) {
        if (godMode) enableGodMode();
    }
    if (ImGui::Checkbox("Instant Tame", &instantTame)) {
        if (instantTame) enableInstantTame();
    }
    if (ImGui::Checkbox("Infinite Stamina", &infiniteStamina)) {
        if (infiniteStamina) enableInfiniteStamina();
    }
    if (ImGui::Checkbox("Rapid Fire", &rapidFire)) {
        if (rapidFire) enableRapidFire();
    }
    ImGui::End();

    ImGui::Render();
    glViewport(0, 0, 1920, 1080);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

extern "C" JNIEXPORT void JNICALL
Java_com_four_direction_ModMenu_startOverlay(JNIEnv *env, jobject thiz) {
    if (!isGameRunning()) {
        LOGD("Game is not running. Launching now...");
        launchGame();
        sleep(5);
    }

    LOGD("Game is running. Starting mod menu...");
    InitMemoryAddresses();
    initOpenGL();

    ImGui::CreateContext();
    ImGui_ImplAndroid_Init(env);
    ImGui_ImplOpenGL3_Init("#version 100");

    while (true) {
        renderMenu();
        usleep(16000);
    }
}
