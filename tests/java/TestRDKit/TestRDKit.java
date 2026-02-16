import org.RDKit.*;

public class TestRDKit {
    static {
        System.loadLibrary("GraphMolWrap");
    }

    public static void main(String[] args) {
        ROMol mol = RWMol.MolFromSmiles("CCO");
        System.out.println("Atoms: " + mol.getNumAtoms());
    }
}
