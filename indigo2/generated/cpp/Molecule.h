class Molecule : public IndigoObject 
{
public:
    static std::shared_ptr<Molecule> createFromMolFile(std::string string);
    
    void aromatize();
    
    std::string molfile();
}

