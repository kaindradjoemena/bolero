// utils/gpu_timer.hpp
#pragma once

#include <glad/glad.h>


namespace blr::utils
{


class GpuTimer
{
public:
    GpuTimer()
    {
        glGenQueries(2, m_queries);

        for (int i = 0; i < 2; i++)
        {
            glBeginQuery(GL_TIME_ELAPSED, m_queries[i]);
            glEndQuery(GL_TIME_ELAPSED);
        }
    }

    ~GpuTimer()
    {
        glDeleteQueries(2, m_queries);
    }

    void Start()
    {
        glBeginQuery(GL_TIME_ELAPSED, m_queries[m_frontBuffer]);
    }

    void Stop()
    {
        glEndQuery(GL_TIME_ELAPSED);
    }

    float GetElapsedMs()
    {
        GLuint backBuffer = 1 - m_frontBuffer;

        GLint available = 0;
        glGetQueryObjectiv(m_queries[backBuffer], GL_QUERY_RESULT_AVAILABLE, &available);

        if (available)
        {
            GLuint64 timeElapsedNs = 0;
            glGetQueryObjectui64v(m_queries[backBuffer], GL_QUERY_RESULT, &timeElapsedNs);
            
            m_lastTimeMs = static_cast<float>(timeElapsedNs) / 1000000.0f;
        }

        m_frontBuffer = backBuffer;

        return m_lastTimeMs;
    }

private:
    GLuint m_queries[2];
    int    m_frontBuffer = 0;
    float  m_lastTimeMs  = 0.0f;
};


} /* namespace blr::utils */