#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // TODO (DONE): (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};

            skyPipelineState.depthTesting.enabled = true;
            skyPipelineState.depthTesting.function = GL_LEQUAL;

            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;

            skyPipelineState.setup();

            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            // TODO (DONE): (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);

            // TODO (DONE): (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).
            colorTarget = new Texture2D();
            colorTarget->bind();

            GLsizei levelsCnt = (GLsizei)glm::floor(glm::log2((float)glm::max(windowSize.x, windowSize.y))) + 1;
            glTexStorage2D(GL_TEXTURE_2D, levelsCnt, GL_RGBA8, windowSize.x, windowSize.y);

            depthTarget = new Texture2D();
            depthTarget->bind();

            glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, windowSize.x, windowSize.y);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);

            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            // TODO (DONE): (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            for (const auto &shader : config["postprocess"])
            {
                ShaderProgram *postprocessShader = new ShaderProgram();
                postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
                postprocessShader->attach(shader.get<std::string>(), GL_FRAGMENT_SHADER);
                postprocessShader->link();
                postprocessShaders.push_back(postprocessShader);
            }
            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShaders[0];
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world)
    {
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lightCommands.clear();
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else if (command.material->affectedByLight)
                {
                    lightCommands.push_back(command);
                }
                else
                {
                    // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        // TODO (DONE): (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        //  HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        Entity *owner = camera->getOwner();
        glm::mat4 M = owner->getLocalToWorldMatrix();

        glm::vec3 eye = M * glm::vec4(0, 0, 0, 1);
        glm::vec3 center = M * glm::vec4(0, 0, -1, 1);
        glm::vec3 cameraForward = glm::normalize(center - eye);

        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
            //TODO (DONE): (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second".
            return glm::dot(cameraForward, first.center) > glm::dot(cameraForward, second.center); });

        // TODO (DONE): (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 viewMatrix = camera->getViewMatrix();
        glm::mat4 projectionMatrix = camera->getProjectionMatrix(windowSize);
        glm::mat4 VP = projectionMatrix * viewMatrix;

        // TODO (DONE): (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        glViewport(0, 0, windowSize.x, windowSize.y);

        // TODO (DONE): (Req 9) Set the clear color to black and the clear depth to 1
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);

        // TODO (DONE): (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            // TODO (DONE): (Req 11) bind the framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
        }

        // TODO (DONE): (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // TODO (DONE): (Req 9) Draw all the opaque commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (const auto &command : opaqueCommands)
        {
            command.material->setup();
            glm::mat4 MVP = VP * command.localToWorld;
            command.material->shader->set("transform", MVP);
            command.mesh->draw();
        }

        int light_count = world->light_count;
        Light *lights = world->lights;

        for (auto &command : lightCommands)
        {
            command.material->setup();

            glm::mat4 M = command.localToWorld;
            glm::mat4 M_IT = glm::transpose(glm::inverse(M));
            glm::vec3 eye = camera->getOwner()->localTransform.position;
            glm::vec3 sky_top = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 sky_horizon = glm::vec3(0.5f, 0.5f, 0.5f);
            glm::vec3 sky_bottom = glm::vec3(0.5f, 0.0f, 0.0f);

            command.material->shader->set("M", M);
            command.material->shader->set("VP", VP);
            command.material->shader->set("M_IT", M_IT);
            command.material->shader->set("eye", eye);
            command.material->shader->set("sky.top", sky_top);
            command.material->shader->set("sky.horizon", sky_horizon);
            command.material->shader->set("sky.bottom", sky_bottom);
            command.material->shader->set("light_count", light_count);

            for (int i = 0; i < light_count; ++i)
            {
                command.material->shader->set("lights[" + std::to_string(i) + "].type", lights[i].kind);
                command.material->shader->set("lights[" + std::to_string(i) + "].position", lights[i].position);
                command.material->shader->set("lights[" + std::to_string(i) + "].color", lights[i].color);
                command.material->shader->set("lights[" + std::to_string(i) + "].attenuation", lights[i].attenuation);
                command.material->shader->set("lights[" + std::to_string(i) + "].direction", lights[i].direction);
                command.material->shader->set("lights[" + std::to_string(i) + "].cone_angles", lights[i].cone_angles);
            }

            command.mesh->draw();
        }

        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // TODO (DONE): (Req 10) setup the sky material
            this->skyMaterial->setup();

            // TODO (DONE): (Req 10) Get the camera position
            glm::vec3 cameraPosition = eye;

            // TODO (DONE): (Req 10) Create a model matrix for the sy such that it always follows the camera (sky sphere center = camera position)
            our::Transform skyTransformer;
            skyTransformer.position = cameraPosition;
            glm::mat4 skyModel = skyTransformer.toMat4();

            // TODO (DONE): (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            //  We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);

            // TODO (DONE): (Req 10) set the "transform" uniform
            skyMaterial->shader->set("transform", alwaysBehindTransform * VP * skyModel);

            // TODO (DONE): (Req 10) draw the sky sphere
            skySphere->draw();
        }
        // TODO (DONE): (Req 9) Draw all the transparent commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (const auto &command : transparentCommands)
        {
            command.material->setup();
            glm::mat4 MVP = VP * command.localToWorld;
            command.material->shader->set("transform", MVP);
            command.mesh->draw();
        }

        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // TODO (DONE): (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            // TODO (DONE): (Req 11) Setup the postprocess material and draw the fullscreen triangle
            postprocessMaterial->setup();
            glBindVertexArray(postProcessVertexArray);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

    void ForwardRenderer::applyPostprocessShader()
    {
        // applyingPostProcess = true;
    }

    void ForwardRenderer::changePostprocessShaderMode(int index)
    {
        postprocessMaterial->shader = postprocessShaders[index];
    }

}