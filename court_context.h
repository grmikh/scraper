#pragma once
#include <stdexcept>

// This context is static but initialized on request. We don't have destructors because it lives forever
class CourtContext {
public:
    static void initContext(const std::string& file) {
        std::call_once(
            m_flag,
            [&file] () { m_instance = new CourtContext(file); }
        );
    }
    static std::string_view getUrl() {
        checkInitialized();
        return m_instance->m_url;
    }

    static const std::set<std::string>& getCourts() {
        checkInitialized();
        return m_instance->m_courts;
    }

    static const std::unordered_map<std::string, std::string>& getCourtsMapping() {
        checkInitialized();
        return m_instance->m_courts_mapping;
    }

private:
    static inline CourtContext* m_instance = nullptr;
    static inline std::once_flag m_flag;
    const std::string m_url;
    std::set<std::string> m_courts;
    std::unordered_map<std::string, std::string> m_courts_mapping;

    static void checkInitialized() {
        if (!m_instance) {
            throw std::logic_error("Context must be initialized before use");
        }
    }
    CourtContext(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            exit( 1 );
        }

        std::string line;

        // Read the file line by line
        while (std::getline(file, line)) {
            std::istringstream lineStream(line);
            std::string key, value;

            // Split the line by '='
            if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
                m_courts_mapping[key] = value;
                m_courts.insert(key);
            } else {
                std::cerr << "Warning: Skipping invalid line: " << line << std::endl;
            }
        }

        file.close();
    }
};
