#ifndef STUDENTBATCH_HPP
#define STUDENTBATCH_HPP

#include <string>

enum class ProgramType { BCS, BCE, BBA, BSE };

class StudentBatch {
    private:
        std::string m_batchId;
        int m_strength;
        ProgramType m_program;
    
    public:
        StudentBatch(std::string batchId, int strength, ProgramType program);
        
        std::string getBatchId() const { return m_batchId; }
        int getStrength() const { return m_strength; }
        ProgramType getProgram() const { return m_program; }
        std::string getProgramAsString() const;
};

#endif