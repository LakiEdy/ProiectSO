abstract class Greutate
{
    abstract public int capacitate();
}
 
class Greutate_simpla extends Greutate
{
    private int capacitate;
 
    public Greutate_simpla(int Capacitate)
    {
        this.capacitate = Capacitate;
    }
 
    public int capacitate()
    {
        return capacitate;
    }
}
 
class Greutate_dubla extends Greutate
{
    private Greutate greutate1;
    private Greutate greutate2;
 
    public Greutate_dubla(Greutate g1, Greutate g2)
    {
        this.greutate1 = g1;
        this.greutate2 = g2;
    }
 
    public void setGreutate1(Greutate g)
    {
        this.greutate1 = g;
    }
 
    public void setGreutate2(Greutate g)
    {
        this.greutate2 = g;
    }
 
    public int capacitate()
    {
        return greutate1.capacitate() + greutate2.capacitate();
    }
}
 
class Greutate_multipla extends Greutate
{
    private Greutate greutati[] = new Greutate[100];
    private int nr_g = 0;
 
    public Greutate_multipla(Greutate[] greutati)
    {
        for(int i =0 ; i < greutati.length ; i++)
        {
            if(greutati[i] != null)
            {
                this.greutati[i] = greutati[i];
                nr_g ++;
            }
            else
            {
                break;
            }
        }
    }
 
    public int capacitate()
    {
        int capacitate = 0;
 
        for(int i=0; i<nr_g; i++)
        {
            capacitate = capacitate + greutati[i].capacitate();
        }
 
        return capacitate;
 
    }
 
    public void adauga(Greutate g)
    {
        greutati[nr_g] = g;
        nr_g ++;
    }
}
 
class ColectieGreutati
{
    private Greutate colectie_greutati[] = new Greutate[100];
    private int nr_g = 0;
    private int SumaGreutati = 0;
 
    public void adauga(Greutate g)
    {
        colectie_greutati[nr_g] = g;
        nr_g ++;
    }
 
    public int Suma()
    {
        for(int i=0; i<nr_g; i++)
        {
            SumaGreutati = SumaGreutati + colectie_greutati[i].capacitate();
        }
 
        return SumaGreutati;
    }
 
    public double medie()
    {
        double rezultat = 0;
 
        if(nr_g != 0)
        {
            rezultat = Suma()/ (double)nr_g;
 
            return rezultat;
        }
 
        return 0;
    }
}
 
class MainGreutate
{
    public static void main(String[] args)
    {
        ColectieGreutati colectie = new ColectieGreutati();
 
        Greutate g1,g2,g3;
 
        g1 = new Greutate_simpla(20);
        g2 = new Greutate_simpla(15);
        g3 = new Greutate_dubla(g1,g2);
 
        Greutate greutati[] = new Greutate[4];
        greutati[0] = g1;
        greutati[1] = g2;
        greutati[2] = g3;
 
        Greutate_multipla g4 = new Greutate_multipla(greutati);
        Greutate_simpla g5 = new Greutate_simpla(6);
 
        g4.adauga(g5);
 
        colectie.adauga(g1);
        colectie.adauga(g2);
        colectie.adauga(g3);
        colectie.adauga(g4);
        colectie.adauga(g5);
 
        System.out.println("Greutatea medie a colectiei este: " + colectie.medie());
 
 
    }
}