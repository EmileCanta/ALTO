//-------------------------------------------------------------------------//
//                                                                         //
//                            Convert_Narval.cxx                           //
//                               Version 1.8                               //
//                        Matthieu Lebois November 2007                    //
//                            with G. Georgiev                             //
//                                                                         //
//     This file contain the function of conversion to root file.          //
//                                                                         //
//-------------------------------------------------------------------------//

// Call for standard libraries
#include "TMath.h"

// Call of all the libraries used in this program
#include "Convert.h"
#include "Fonctions_convert.h"

// Definition of I/O function
using namespace std;

//Definition of a debugging function
int Coucou(int i)
{
  cout << "Coucou I'm here " << i << endl;
  return 0;
}

//Definition of the function to reconstitute the time
#define TEMPS_47bits(x,y,z) (Double_t)( ((Double_t)(x & 0x7fff) * 65536.0 * 65536.0) + ((Double_t)y * 65536.0) + (Double_t)z )

// Definition of conversion function
int convertfile(const char* paramfilename,const char *inputfilename, const char *outputfilename,UChar_t coding, UChar_t swap)
{
  
  // Declaration of variable for the construction of a hit
  UChar_t       gr;                  // group
  UChar_t       sl;    	             // slot
  UChar_t       cr;	             // crate
  UChar_t       ser;                 // service
  UChar_t       mark;                // mark
  UChar_t       ch;	             // channel
  UShort_t      en;	             // energy
  Double_t      tm;	             // time that will also be stored in the tree
  UChar_t       marker;              // marker to store in the tree
  UInt_t        enrj;                // Energy to store in the tree
  UInt_t        codingenable = 0;    // Counter of coding enable.
  
  // Declaration of the different pointers used during the conversion
  char * word1;
  char * word2;
  char * bf16;		      // used to read 16 bit blocks
  //char branchname1[32];
  char nomdetector[32];
  //TString *branchdetname;     // Branch names
  
  // Declaration of the others variable used during the conversion
  unsigned short bf;
  unsigned int   tm1 = 0;              // Middle part of time
  unsigned int   tm2 = 0;              // Lower part of time
  unsigned int   tm3 = 0;              // Higher part of time
  int            chks=0;
  ULong64_t      n = 0;		       // event counter max 4 billion if unsigned long
  Long64_t       nbrevent = 0;         // Counter to know the real numbers of entries in the tree
  int            nbreventfous = 0;     // Counter to count event with no meaning
  int            nbrevent2 = 0;        // Counter to select a good number of event
  int            memory_nbrevent_coding = 0; // variable pour stocker le numero d'evenement physique precedent le coding enable
  int            buffersize = 1000000;  // Size of the buffer that will be manipulated
  int            nbrbuffer = 0;        // Countenr on the number of buffer treated
  int            n0det;
  TString        type;
  int            id;

  int            group;
  int            crate;
  int            slot;
  int            codenvu = 0;
  UChar_t        index;
  int            channel;
  int            nbrdetect;
  int            nbrdesordre = 0;
  int            smallnbrcoding = 0;   // Counter to check the number of coding_enable (one/card)
  int            nbrcoding = 0;        // Counter to count the number of coding enable
  int            nbrcard = 0;          // Memorization of the number of cards to check the coding enable
  int            position1erevenement_postcoden = 0;
  int            positionderniererevenement_postcoden = 0;
  int            dimtab_coden = 0;
  
  // Dynamic allocation of the pointeurs that will contain the data read from the file
  UChar_t  *buffer_marker;
  UInt_t   *buffer_enrj;
  Double_t *buffer_tm;
  UChar_t  *buffer_idx;
  UChar_t  *buffer_marker_coden;
  UInt_t   *buffer_enrj_coden;
  Double_t *buffer_tm_coden;
  UChar_t  *buffer_idx_coden;
  Double_t *tampon_tm;
  UInt_t   *tampon_enrj;
  UChar_t  *tampon_marker;
  UChar_t  *tampon_idx;

	
  // Compteur pour le nombre de coup par detecteur!
  UInt_t *nbrcoup_detect;
  UInt_t *nbrcoup_detect_saved;
  UInt_t *nbrcoup_detect_test;
	
	
	
	
  //-----------------------------------------------------------------------------//
  //                                                                             //
  //         Part that is used to define the parameters that belong to the       //
  //         experiment: definition of an index to recognize the differents      //
  //         parameters.                                                         //
  //                                                                             //
  //-----------------------------------------------------------------------------//
	
  // Definition of a structure to create an tree as an index between
  // group/slot/channel/crate and a detector's number
  struct detector
  {
    UChar_t  nomdetect;
  }det_num;
	
  //Declaration of variables to read parameters
Coucou(666);
  TString nomFichierParamLect;
Coucou(667);
  std::string buffer;
Coucou(668);
  nomFichierParamLect = paramfilename; 
Coucou(669);  
// Declaration of pointers for the reading of files
  //ifstream fich_paramlect(nomFichierParamLect.Data(), ios::in);
ifstream fich_paramlect(nomFichierParamLect.Data());
Coucou(670);
  
  // Declaration of the tree that will be used to store the parameters
  //  TTree *input = new TTree("input","Parameters tree");

  int idx[5][6];
  for (int i=1; i<6;i++) {
    for(int j = 0; j<6;j++){
      idx[i][j]=0;
    }
  }
  while(fich_paramlect.good()){
    std::getline(fich_paramlect,buffer);
    TString junk;
    if(buffer.empty()) continue;
    if(buffer[0]=='#') continue;
    std::istringstream line(buffer);
    if(buffer.find("NbrOfCOMETCards")!=std::string::npos) {
      line >> junk >> nbrcard ;
      std::cout << "Number of COMET Cards: " << nbrcard << std::endl;
    }
    else if(buffer.find("NbrOfDetectors")!=std::string::npos) {
      line >> junk >> nbrdetect ;
      std::cout << "Number of Detectors: " << nbrdetect << std::endl;
    }
    else {
      line >> n0det >> type >> id  >> group >> crate >> slot >> channel;
      std::cout << n0det << "  " 
		<< type  << "  "
		<< id    << "  "
		<< crate << "  "
		<< slot  << "  "
		<< channel << std::endl;
      if(n0det){
	idx[slot][channel]=(UInt_t)n0det;
	  
      }
    }
  }

	
  // Initialisation de la memoire pour le tableau de compteur
  nbrcoup_detect = new UInt_t[nbrdetect+1];
  nbrcoup_detect_saved = new UInt_t[nbrdetect+1];
  nbrcoup_detect_test = new UInt_t[nbrdetect+1];
  for(int i = 0; i < nbrdetect+1; i++)
    {
      nbrcoup_detect[i] = 0;
      nbrcoup_detect_saved[i] = 0;
      nbrcoup_detect_test[i]=0;
    }
  //-----------------------------------------------------------------------------//
  //                                                                             //
  //         Part that is used to define the parameters that belong to the       //
  //         experiment: definition of an index to recognize the differents      //
  //         parameters.                                                         //
  //                                                                             //
  //-----------------------------------------------------------------------------//
  
  // Memory allocation for a buffer to check the event are in order
  buffer_marker = new UChar_t[buffersize];
  buffer_idx = new UChar_t[buffersize];
  buffer_enrj = new UInt_t[buffersize];
  buffer_tm = new Double_t[buffersize];
  
  tampon_tm = new Double_t[nbrdetect+1];
  tampon_enrj = new UInt_t[nbrdetect+1];
  tampon_marker = new UChar_t[nbrdetect+1];
  tampon_idx = new UChar_t[nbrdetect+1];
	
  buffer_marker_coden = new UChar_t[(int)(4*nbrdetect+1)];
  buffer_idx_coden = new UChar_t[(int)(4*nbrdetect+1)];
  buffer_enrj_coden = new UInt_t[(int)(4*nbrdetect+1)];
  buffer_tm_coden = new Double_t[(int)(4*nbrdetect+1)];

  for(int j = 0; j < (4*nbrdetect+1); j++){
    buffer_marker_coden[j] = 0;
    buffer_idx_coden[j] = 0;
    buffer_enrj_coden[j] = 0;
    buffer_tm_coden[j] = 0;
  }
  
  // Initialisation des tampons
  for(int j = 0; j < nbrdetect+1; j++){
    tampon_tm[j] = 0;
    tampon_enrj[j] = 0;
    tampon_marker[j] = 0;
    tampon_idx[j] = 0;
  }	
	
  //-----------------------------------------------------------------------------//
  //                                                                             //
  //             Creation of the ouput file and of the output tree               //
  //  The tree will contain:                                                     //
  //     - The detector number (UChar_t)                                         //
  //     - The energy (Int_t)                                                    //
  //     - The time (Double_t)                                                   //
  //     - Marker (UChar_t)                                                      //
  //     - Coden (counter: Int_t)                                                //
  //                                                                             //
  //-----------------------------------------------------------------------------//
	
  // Declaration of the root file used to save data
  //TFile *outputfile = new TFile(outputfilename,"RECREATE");
  TFile outputfile(outputfilename,"RECREATE");
	
  // Opening data file
  ifstream narval ;
  cout << " name of file " << inputfilename << endl;
  narval.open(inputfilename, ios::in|ios::binary|ios::ate);
	
  // Declaration of oak, that is the tree containing all events
  cout<< "Building tree  " <<endl;
  TTree *oak = new TTree("Narval_tree",inputfilename);
	
  // Declaration of Branches that will contain the data
  oak->Branch("Det_nbr",&index,"index/b");  // Detector number
  oak->Branch("Energy",&enrj,"enrj/i");     // Energy coded on the channel
  oak->Branch("Time",&tm,"tm/D");           // Time of the hit
  oak->Branch("Marker",&marker,"marker/b"); // Marker of the hit (0 if no marker and 1 if marker)
  oak->Branch("Type",&type,"type/C");       // Detector Type     
  oak->Branch("DetId",&id,"id/i");          // Detector Id
	
  // I create this branch if there is a coden
  if(coding == 1)
    {
      oak -> Branch("Coding", &codingenable, "codingenable/i");
    }
  
  // In case of huge tree I set an autosave to free some space in the memory
  oak -> SetAutoSave(10*buffersize);
	
  cout << "Seed burried " << endl;
  cout <<"  and in the memory a tree will grow..."<<endl ;
	
	
	
  //-----------------------------------------------------------------------------//
  //                                                                             //
  //      Part that read the file:                                               //
  //       I always search for a coden (in case)                                 //
  //       The method is available for both cases                                //
  //                                                                             //
  //-----------------------------------------------------------------------------//
  
  // I try to forsee the size of the output file
  streampos inputfilesize = narval.tellg();
  cout << "Filesize is " << inputfilesize << " bytes for "<< inputfilename << endl;
  
  // Permission for a root file to exceed 2GB
  if(inputfilesize > 2000000000)
    {
      oak->SetMaxTreeSize(2000000000);
    }
  cout << "The root file size will be of ~ " << (Double_t)inputfilesize/2000000000 << " Gb"<< endl;
  
	
  // I check for the quality of the file (bad writing)
  narval.seekg(0,ios::beg);
  if (inputfilesize < 0 )
    {
      // cout <<"$$$"<<endl <<" $$$ Problem with file "<<inputfilename<<endl ;
      //       cout << "$$$ skiping it"<<endl<<"$$$ "<<endl ;
      return 3;
    }
  
  // Skip header till we find an event 0xffff 0x8
  word1 = new char [2];
  word2 = new char [2];
  bf16 = new char [2];
  narval.read(word1,2);
  narval.read(word2,2);
  
  while ((word(swap,word1[0],word1[1]) != 0xffff)||(word(swap,word2[0],word2[1]) !=0x8)){
    // Move one word backwards but read two
    narval.seekg(-2,ios::cur);
    if (narval.good()) narval.read(word1,2);
    if (narval.good()) narval.read(word2,2);
		
    // Test to check the end of file (corrupted file)
    if (narval.eof()){
      cout << "Fatal error, file does not start with detector event" << endl;
      return n;
    }// end of if (narval.eof())
  }// end of while ((word(swap,word1[0],word1[1]) != 0xffff)||(word(swap,word2[0],word2[1]) !=0x8))
  
  // Writting of the declaration for the good start of conversion
  cout << "Start conversion at position " << narval.tellg()-(streampos)4<< endl;
  cout << "-------------------------------------------------"<< endl;
  
  // Scan of all the data file
  // I set to 0 all the counters
  n = 0;
  smallnbrcoding = 0;
  nbrcoding = 0;
  nbrevent = 0;
  nbrevent2 = 0;
  nbrbuffer = 0;
  codingenable = 0;
  nbreventfous = 0;
	
  // Start reading the physical events
  while (narval.good()){
    
    // Check for a physical event: detector event 0xffff 0x0008
    while ((word(swap,word1[0],word1[1])==0xffff) && (word(swap,word2[0],word2[1])==0x8)){
	
      // Increment of read event counter
      n++;
	      
      // Extraction of hit description
      if (narval.good()) narval.read(bf16,2);
	      
      // Read group, slot, crate, ch, marker and service
      bf = word(swap,bf16[0],bf16[1]);
      gr = (UChar_t)(bf >> 12);
      sl = (UChar_t)((bf >> 8) & 15);
      cr = (UChar_t)((bf >> 6) & 3);
      ser = (UChar_t)((bf >> 4 ) & 2);
      mark = (UChar_t)((bf >> 4 ) & 1);
      ch = (UChar_t)(bf & 15);
				
      // Creation of the detector number to find it in the index
      sprintf(nomdetector,"idx_detect_%d_%d_%d_%d",gr,cr,sl,ch);
	      
      // Read energy
      if (narval.good()) narval.read(bf16,2);
      en = (UInt_t)word(swap,bf16[0],bf16[1]);
				
      // Read and reconstruct time
      if (narval.good()) narval.read(bf16,2);
      tm1 = (word(swap,bf16[0],bf16[1]));
      if (narval.good()) narval.read(bf16,2);
      tm2 = word(swap,bf16[0],bf16[1]);
      if (narval.good()) narval.read(bf16,2);
      chks = word(swap,bf16[0],bf16[1]);
      if (narval.good()) narval.read(bf16,2);
      tm3 = (word(swap,bf16[0],bf16[1]));
	      
      // Time reconstruction to be able to use it for checking
      tm = TEMPS_47bits(tm3,tm1,tm2); // the time is given in ps
				
      //-----------------------------------------------------//
      //   I start checking that my event is not a coden     //
      //-----------------------------------------------------//
	      
      // Verification of the presence of a coding enable to reset the memory of time
      // A coding enable event is define as a service event on a channel 6 (unexisting channel)
      // A there is one per card
      if(ser == 2 && ch == 6){// coden event is ffff 0008 0y26 with y the card number
	
	smallnbrcoding++;
					
	//cout << "j'ai vu un coden !!!" << codingenable << endl;
	//cout << "Tm = " << (double) tm*400 << endl; // tm en ps
	//cout << "card = " << (int) sl << endl;
	memory_nbrevent_coding = nbrevent2;
	//cout << "j'ai vu un coden !!!" << codingenable << endl;
	//cout << "@position : " << memory_nbrevent_coding << endl;
	//cout << "Tm = " << (double) tm << endl;
	//cout << "TM before = : " << (double)buffer_tm[nbrevent2] << endl;
	//cout << "card = " << (int) sl << endl;
					
	// I check I've got all the coden of each card
	if(smallnbrcoding == nbrcard){
		
	  // Once I've got the same number of card and coden,
	  // I create one event that is a coden
	  smallnbrcoding = 0;
	  nbrcoding++;
	  //memory_nbrevent_coding = nbrevent2;
	  codenvu = 1;
						
	    cout << "j'ai vu un coden !!!" << codingenable << endl;
	  // cout << "@position : " << memory_nbrevent_coding << endl;
						
	  //cout << "Tm = " << (double) tm << endl;
	  // cout << "TM before = : " << (double)buffer_tm[nbrevent2] << endl;
	  // cout << "card = " << (int) sl << endl;
						
	  // // Coden event is saved in the table
	  // 		  buffer_idx[nbrevent2] = 0;
						
	  // 		  // Filling of the structure to prepare the storing of the event
	  // 		  buffer_marker[nbrevent2] = 0;
	  // 		  buffer_enrj[nbrevent2] = -666;
	  // 		  buffer_tm[nbrevent2] = -666; // to be in unit of ps time of the last coden
						
	  // 		  // Augmentation of hit counter
	  // 		  nbrevent++;
	  // 		  nbrevent2++;
						
	}// End of if(smallnbrcoding == nbrcard)
      }// End of if(ser == 1 && mark == 1 && ch == 5)
				
				
      //-----------------------------------------------------//
      //          This event is not a service event          //
      //            I keep this one an work on it            //
      //-----------------------------------------------------//
	      
      // Filling of the tree used for storing PHYSICAL hits
      if(ser == 0){ // checking it is no a service hit
	    
	//----------------------------------------------//
	//     I start checking this detector exists    //
	//----------------------------------------------//
					
	// Creation of the detector number to find it in the index (x2 pour etre tranquille)
	sprintf(nomdetector,"idx_detect_%d_%d_%d_%d",gr,cr,sl,ch);
	sprintf(nomdetector,"idx_detect_%d_%d_%d_%d",gr,cr,sl,ch);
	//printf(nomdetector,"idx_detect_%d_%d_%d_%d",gr,cr,sl,ch);
	//cout << nomdetector << endl;

	// Reading of the input tree to get the information concerning the detector number
	//TBranch *idx = input->GetBranch(nomdetector);
	//cout<<input->GetBranch(nomdetector)<<endl;
	// Check for the quality of the information read
	//std::cout << "Slot: " << (UInt_t)sl << "   Channel: " << (UInt_t)ch << "   idx=" << idx[sl][ch] << std::endl;
	if(idx[sl][ch]>0){
	 

	  // Reading of the index to get the corresponding detector number corresponding
	  //idx -> SetAddress(&det_num); //##??
	  //idx -> GetEntry(0);
							
	  // I fill the buffers of event
	  buffer_idx[nbrevent2] = (UChar_t)idx[sl][ch];//det_num.nomdetect;
	  buffer_marker[nbrevent2] = mark;
	  buffer_enrj[nbrevent2] = en;
	  buffer_tm[nbrevent2] = tm*400; // to be in unit of ps
							
	  if(tm > 1.e-8 
	     && (buffer_idx[nbrevent2] < nbrdetect + 1) 
	     && (buffer_idx[nbrevent2] != 0)) nbrcoup_detect[det_num.nomdetect]++;
						
							

	  //if(nbrevent2 == memory_nbrevent_coding +1 ){
	  //  cout << "Old time : " <<  (double)buffer_tm[(int)(memory_nbrevent_coding)] << endl;
	  //  cout << "New time : " << (double)buffer_tm[nbrevent2] << endl;
	  // }
							
							
	  // Je sors a l'ecran les premier evenements
	  //if(codingenable <2 && nbrevent2 < 9470) cout << "First events : " << nbrevent2 << " buffer_idx = " << (int)buffer_idx[nbrevent2] << "; buffer_marker = " << (int)buffer_marker[nbrevent2] << "; buffer_enrj = " << (double)buffer_enrj[nbrevent2] << "; buffer_tm = " << (double)buffer_tm[nbrevent2] << "; coding enable : " << codingenable << endl;
							
	  // Augmentation of hit counter
	  nbrevent++;
	  nbrevent2++;
							
							
	  //----------------------------------------------------//
	  //   In some specific cases, I will check the order   //
	  //  in the different buffers and save it to the tree  //
	  //----------------------------------------------------//
							
							
	  //------------------------------------------------------------//
	  //              First buffer of nbrdetect event               //
	  //------------------------------------------------------------//
							
	  // In the case of the very first events,
	  // I create a buffer with the size of the "tampon" to fill it
	  // Usually, this buffer start with a coden (even without coden)
	  // So I have to order nbrdetect events
	  if(nbrevent2 == nbrdetect && codingenable == 0){ // It's always the case even with no coden in the experiment
	      
		      		  
	    // I sort this very small buffer of nbrdetect event and fill the "tampon"
	    first_tri_buffers(tampon_tm,buffer_tm,tampon_enrj,buffer_enrj,tampon_marker,buffer_marker,tampon_idx, buffer_idx, nbrevent2, nbrdetect);
								
	    // I increase the number of buffer
	    nbrbuffer++;
								
	    // I change the value of coden to be sure not to get back in this loop (in any case)
	    codingenable = 1;
								
	    // I make nbrevent2 back to zero to count again the number of event to reach the size of a buffer
	    nbrevent2 = 0;
								
	    // I reset the value for the coden to look for it another time
	    memory_nbrevent_coding = -666;
	    codenvu = 0;
	  }
							
							
	  //------------------------------------------------------------//
	  //                     Buffer with a coden                    //
	  //------------------------------------------------------------//
							
	  // Here a coden has been seen... I stop nbrdetect after the coden to be sure to have just one
	  // coden per buffer
	  if((nbrevent2 ==  (memory_nbrevent_coding + 2*nbrdetect)) && codenvu == 1){
	      
	    //cout << "j'ai un coden" << endl;
								
	    // Here I define the size of the buffer
	    int buffersize2 = nbrevent2;
								
	    // I count this buffer
	    nbrbuffer++;
								
	    //cout << endl;
	    //cout << "J'ai vu un coden donc je trie avec un coden " << codingenable << endl;
	    //cout << endl;
								
	    // Pour trier un buffer avec un coden, je le decoupe en plusiers bout:
								
	    //---------------------------------------------
	    // Premier bout: Je trie la partie avec coden
	    //---------------------------------------------
								
	    // Je prends les 2*nbrdetects derniers evenements du buffer et je les tris en pre-coden et post-coden
	    for(int ii = (buffersize2-1); ii > (buffersize2 - 4*nbrdetect-1); ii--){
		  
									
	      buffer_marker_coden[(int)(ii-(buffersize2 - 4*nbrdetect))] = buffer_marker[ii];
	      buffer_idx_coden[(int)(ii-(buffersize2 - 4*nbrdetect))] = buffer_idx[ii];
	      buffer_enrj_coden[(int)(ii-(buffersize2 - 4*nbrdetect))] = buffer_enrj[ii];
	      buffer_tm_coden[(int)(ii-(buffersize2 - 4*nbrdetect))] = buffer_tm[ii];
	      //cout << ii << endl;
	      //cout << (int)(ii-(buffersize2 - 4*nbrdetect)) << endl;
	    }
								
	    // J'ai selectionne les derniers evenements
	    // Il me faut faire un tri pre-coden et post-coden
	    tri_preandpost_coden(buffer_tm_coden,buffer_enrj_coden,buffer_idx_coden,buffer_marker_coden,4*nbrdetect+1,&position1erevenement_postcoden,&positionderniererevenement_postcoden);
								
								
	    // cout << "position1erevenement_postcoden = " << position1erevenement_postcoden << endl;
	    //              cout << "positionderniererevenement_postcoden = " << positionderniererevenement_postcoden << endl;
								
	    //---------------------------------------------
	    // Deuxieme bout: Je trie la partie pre-coden
	    //---------------------------------------------
								
	    // Normalement de 0 \E0 position1erevenement_postcoden-1 j'ai tout les evenements pre coden
	    // Je mets a zero les derniers evenements du buffer
	    for(int ii = (buffersize2-1); ii > (buffersize2 - 4*nbrdetect-1); ii--){
	      buffer_marker[ii] = 0;
	      buffer_idx[ii] = 0;
	      buffer_enrj[ii] = 0;
	      buffer_tm[ii] = 0;
	    }
	    // Puis je remets les evenements tries pre coden dans le buffer pour retrier completement le buffer
	    for(int ii = 0; ii < position1erevenement_postcoden; ii++){	      
	      buffer_marker[(int)(ii+(buffersize2 - 4*nbrdetect))] = buffer_marker_coden[ii];
	      buffer_idx[(int)(ii+(buffersize2 - 4*nbrdetect))]    = buffer_idx_coden[ii];
	      buffer_enrj[(int)(ii+(buffersize2 - 4*nbrdetect))]   = buffer_enrj_coden[ii];
	      buffer_tm[(int)(ii+(buffersize2 - 4*nbrdetect))]     = buffer_tm_coden[ii];
	      //cout << (int)(ii+(buffersize2 - 2*nbrdetect)) << endl;
	      //if(codingenable > 59&& codingenable< 63) cout << ii+(buffersize2 - 4*nbrdetect) << " buffer_marker = " << (int)buffer_marker[(int)(ii+(buffersize2 - 4*nbrdetect))] << "; buffer_idx = " << (int)buffer_idx[(int)(ii+(buffersize2 - 4*nbrdetect))] << "; buffer_enrj = " << (double)buffer_enrj[(int)(ii+(buffersize2 - 4*nbrdetect))] << "; buffer_tm = " << (double)buffer_tm[(int)(ii+(buffersize2 - 4*nbrdetect))] << endl;
	    }
								
	    //cout << "coucou avant tri buffer : "<<(buffersize2 - 2*nbrdetect) + position1erevenement_postcoden <<endl;
	    // Puis je trie le buffer sans le coden, je fais le trie aveec les evenements du tampon precedent
	    tri_buffers(tampon_tm, buffer_tm, tampon_enrj,buffer_enrj,tampon_marker,buffer_marker,tampon_idx, buffer_idx,(buffersize2 - 3*nbrdetect) + position1erevenement_postcoden,nbrdetect);
								
	    // La fonction a trie le buffer et je peux maintenant le mettre dans l'arbre
	    // Now the buffer is sorted I will be able to store it in the TTree
	    for(int ll = 0; ll < ((buffersize2 - 4*nbrdetect)+position1erevenement_postcoden); ll++){
	      
	      //cout << "Euh taille cherchee " << ((buffersize2 - 2*nbrdetect)+position1erevenement_postcoden) << endl;
	      //cout << "Taille max " << buffersize << endl;
	      // I get the information back from the buffer and fill the tree
	      index = buffer_idx[ll];
	      marker = buffer_marker[ll];
	      enrj = buffer_enrj[ll];
	      tm = buffer_tm[ll];
	      if(buffer_tm[ll] < 1.e-8) tm = 0;
									
	      // if(codingenable > 59&& codingenable< 63)cout  << "Put in tree first part coden buffer : " << ll << " buffer_marker = " << (int)buffer_marker[ll] << "; buffer_idx = " << (int)buffer_idx[ll] << "; buffer_enrj = " << (double)buffer_enrj[ll] << "; buffer_tm = " << (double)buffer_tm[ll] << "; coding enable : " << (int)codingenable<< endl;
									
	      nbrcoup_detect_test[index]++;
									
	      if(index == 0 && enrj !=0) cout << "enrj = " << (int)enrj << "; time = " << (double)tm << endl;
									
	      // I fill the tree now
	      if(tm != 0 && (index < nbrdetect + 1) && (index != 0)){
		  
		//Coucou(ll);
		oak -> Fill();
		nbrcoup_detect_saved[index]++;
		//Coucou(666);
	      }
	    }// End of for(int ll = 0; ll < buffersize2; ll++)
								
	    //cout << endl;
	    //cout << endl;
								
	    // Toutefois le tampon contient encore des evenements du coden:
	    for(int ll = 0; ll < 2*nbrdetect+1; ll++)
	      {
		// I get the information back from the buffer and fill the tree
		index = tampon_idx[ll];
		marker = tampon_marker[ll];
		enrj = tampon_enrj[ll];
		tm = tampon_tm[ll];
		if(tampon_tm[ll] < 1.e-8) tm = 0;
									
		//if(codingenable > 59&& codingenable< 63)cout  << "Coden buffer : " << ll << " tampon_marker = " << (int)tampon_marker[ll] << "; tampon_idx = " << (int)tampon_idx[ll] << "; tampon_enrj = " << (double)tampon_enrj[ll] << "; tampon_tm = " << (double)tampon_tm[ll] << endl;
									
		nbrcoup_detect_test[index]++;
									
		// I fill the tree now
		//if(tm != 0 && (index < nbrdetect + 1)) oak -> Fill();
		if((tm != 0) && (index < nbrdetect + 1) && (index !=0))
		  {
		    oak -> Fill();
		    nbrcoup_detect_saved[index]++;
		  }
									
		// I use this loop to reset the "tampon"
		tampon_idx[ll] = 0;
		tampon_marker[ll] = 0;
		tampon_enrj[ll] = 0;
		tampon_tm[ll] = 0;
	      }
								
								
	    // I had a coden so I increase the counter
	    codingenable++;
								
	    //----------------------------------------
	    // Troisieme bout : la partie post-coden
	    //----------------------------------------
								
	    // J'ai un tableau qui contient que des evenements appartenant au coden suivant
	    // Il faut que je les trie et que je les mette dans le tampon
	    // Toutefois s'il y a trop d'evenements pour aller dans le tampon il faut que le surplus des premiers aille dans l'arbre
								
	    // Je decale tout mes evenements post coden vers le debut du tableau
	    for(int ii = 0; ii < 2*nbrdetect; ii++)
	      {
		if(ii+position1erevenement_postcoden < 2*nbrdetect)
		  {
		    buffer_marker_coden[ii] = buffer_marker_coden[(int)(ii+position1erevenement_postcoden)];
		    buffer_idx_coden[ii]    = buffer_idx_coden[(int)(ii+position1erevenement_postcoden)];
		    buffer_enrj_coden[ii]   = buffer_enrj_coden[(int)(ii+position1erevenement_postcoden)];
		    buffer_tm_coden[ii]     = buffer_tm_coden[(int)(ii+position1erevenement_postcoden)];
		    //cout << ii << endl;
										
		    //  if(codingenable > 59&& codingenable< 63) cout << "Post coden part " << ii<< " buffer_marker_coden[" << ii << "] = " << (int)buffer_marker_coden[ii] << "; buffer_idx_coden[" << ii << "] = " << (int)buffer_idx_coden[ii] << "; buffer_enrj_coden[" << ii << "] = " << (int)buffer_enrj_coden[ii] << "; buffer_tm_coden[" << ii << "] = " << (double)buffer_tm_coden[ii]<< endl;
		  }
		else
		  {
		    //cout << ii << endl;
		    //cout << "Est-ce la l'erreur?"<<endl;
		    buffer_marker_coden[ii] = 0;
		    buffer_idx_coden[ii]    = 0;
		    buffer_enrj_coden[ii]   = 0;
		    buffer_tm_coden[ii]     = 0;
		  }
	      }
	    // Je regarde la dimension du tableau pour effectuer un tri et comparer au tampon
	    dimtab_coden = 0;
	    for(int ii = 0; ii < 2*nbrdetect; ii++)
	      {
		if(buffer_tm_coden[ii] == 0)
		  {
		    dimtab_coden = ii;
		    break;
		  }
	      }
								
	    //cout << "dimtab_coden = " << dimtab_coden << endl;
								
	    //dimtab_coden contient l'indice du premier element a zero
	    // Il me faut trier ce tableau
	    tri_buffer_postcoden(buffer_tm_coden, buffer_enrj_coden, buffer_marker_coden, buffer_idx_coden, dimtab_coden+1);
								
	    // J'ai trie le tableau
	    // Deux cas se presentent:
								
	    // 1er cas:
	    // J'ai moins d'evenements (ou autant) post coden que pour un tampon:
	    // Je lis mon tableau et je remplis le reste avec des zeros
	    // Les zeros seront tries la fois suivante et ne seront pas inscrits dans le fichier root
	    if(dimtab_coden+1 <= 2*nbrdetect){
	      
	      for(int ii = 0; ii < nbrdetect; ii++){
		  
		if(ii < dimtab_coden+1){
		      
		  tampon_tm[ii] = buffer_tm_coden[ii];
		  tampon_enrj[ii] = buffer_enrj_coden[ii];
		  tampon_marker[ii] = buffer_marker_coden[ii];
		  tampon_idx[ii] = buffer_idx_coden[ii];
											
		  // if(codingenable > 59&& codingenable< 63)cout  << "post with less event " << ii << " && dimtab_coden+1 : " <<dimtab_coden+1 <<   " tampon_marker = " << (int)tampon_marker[ii] << "; tampon_idx = " << (int)tampon_idx[ii] << "; tampon_enrj = " << (double)tampon_enrj[ii] << "; tampon_tm = " << (double)tampon_tm[ii] << "; codingenable : "<< codingenable << endl;
		}
		else{
		      
		  tampon_tm[ii] = 0;
		  tampon_enrj[ii] = 0;
		  tampon_marker[ii] = 0;
		  tampon_idx[ii] = 0;
		}
	      }
	    }
								
	    // 2eme cas:
	    // J'ai plus d'evenements post coden qu'il n'en faut pour le tampon
	    // Je vais mettre les premiers qui sont en trop dans l'arbre et le reste dans le tampon
	    // Je lis les evenements par la fin et je remplis le tampon par la fin
	    else if(dimtab_coden+1 > 2*nbrdetect){
	      
	      for(int ii = dimtab_coden; ii > (dimtab_coden - nbrdetect); ii--){
		  
		tampon_tm[ii] = buffer_tm_coden[ii];
		tampon_enrj[ii] = buffer_enrj_coden[ii];
		tampon_marker[ii] = buffer_marker_coden[ii];
		tampon_idx[ii] = buffer_idx_coden[ii];
	      }
									
	      // Puis je relis dans le bon ordre les evenements pour les mettre dans l'arbre
	      for(int ii = 0; ii < (dimtab_coden - nbrdetect + 1); ii++){
		  
										
		//cout << "dimtab_coden - nbrdetect + 1 " << dimtab_coden - nbrdetect + 1 << endl;
		//cout << "2*nbrdetect+1 " << 2*nbrdetect+1 << endl;
		//cout << "ii actuel : " << ii << endl;
										
		// I get the information back from the buffer and fill the tree
		index = buffer_idx_coden[ii];
		marker = buffer_marker_coden[ii];
		enrj = buffer_enrj_coden[ii];
		tm = buffer_tm_coden[ii];
		if(buffer_tm_coden[ii] < 1.e-8) tm = 0;
										
										
		//if(codingenable > 59&& codingenable< 63)cout  << "post coden with more event : " << ii << "buffer_marker_coden  = " << (int)buffer_marker_coden[ii] << "; buffer_idx_coden = " << (int)buffer_idx_coden[ii] << "; buffer_enrj_coden = " << (double)buffer_enrj_coden[ii] << "; buffer_tm_coden = " << (double)buffer_tm_coden[ii] << endl;
										
		nbrcoup_detect_test[index]++;
		// I fill the tree now
		//if(tm != 0 && (index < nbrdetect + 1)) oak -> Fill();
		if(tm != 0 && (index < nbrdetect + 1) && (index != 0))
		  {
		    //Coucou(1);
		    oak -> Fill();
		    nbrcoup_detect_saved[index]++;
		  }
	      }
	    }
								
								
	    // Mon tampon est plein
	    // L'arbre avec la nouvelle cession de coden est remplie
	    // Donc :
								
	    // I make nbrevent2 back to zero to count again the number of event to reach the size of a buffer
	    nbrevent2 = 0;
								
	    // I reset the value for the coden to look for it another time
	    memory_nbrevent_coding = -666;
	    codenvu = 0;
								
	    // And I reset the table (to be sure)
	    for(int j = 0; j < (2*nbrdetect); j++)
	      {
		buffer_marker_coden[j] = 0;
		buffer_idx_coden[j] = 0;
		buffer_enrj_coden[j] = 0;
		buffer_tm_coden[j] = 0;
	      }
								
	    //cout << "J'ai fini de trier le coden : " << codingenable-1 << endl;
	  }// End of if((nbrevent2 ==  (memory_nbrevent_coding + nbrdetect)) && codenvu == 1)
							
							
	  //------------------------------------------------------------//
	  //                   Buffer without a coden                   //
	  //------------------------------------------------------------//
							
	  // I have no coden in this buffer so I make a standard sorting
	  // This function will be use, the most often, in the case of no coden
	  if(nbrevent2 == buffersize && codenvu != 1)
	    {
	      //cout << "Un nouveau buffer d'evenements a trier: Youpi! " << nbrbuffer << endl;
								
	      //if(cout << "preparation buffer sans coden" << endl;
	      // Tri des buffers
	      tri_buffers(tampon_tm, buffer_tm, tampon_enrj,buffer_enrj,tampon_marker,buffer_marker,tampon_idx, buffer_idx, buffersize, nbrdetect);
								
	      //cout << "buffer sans coden" << endl;
								
	      //if(nbrbuffer > 2220 && nbrbuffer < 2230) Coucou(1);
								
	      // I increase the number of buffer
	      nbrbuffer++;
								
	      // Now the buffer is sorted I will be able to store it in the TTree
	      for(int ll = 0; ll < buffersize; ll++)
		{
		  // I get the information back from the buffer
		  index = buffer_idx[ll];
		  marker = buffer_marker[ll];
		  enrj = buffer_enrj[ll];
		  tm = buffer_tm[ll];
		  if(buffer_tm[ll] < 1.e-8) tm = 0;
									
		  nbrcoup_detect_test[index]++;
									
		  // I store the information in the tree
		  //if(tm != 0 && (index < nbrdetect + 1)) oak -> Fill();
		  if(tm != 0 && (index < nbrdetect + 1) && (index != 0))
		    {
		      oak -> Fill();
		      nbrcoup_detect_saved[index]++;
		    }
		  //oak -> AutoSave();
		}
								
	      //if (nbrbuffer > 2220 && nbrbuffer < 2230) Coucou(2);
								
	      // I make nbrevent2 back to zero to count again the number of event to reach the size of a buffer
	      nbrevent2 = 0;
								
	      // I reset the value for the coden to look for it another time
	      memory_nbrevent_coding = -666;
	      codenvu = 0;
								
	    }// End of if(nbrevent2 == buffersize)
	}// End of if(idx)
					
				
	// Case of an event that is not contained in the index tree
	// It is not supposed to have to many of them
	else
	  {

	    //printf(nomdetector,"idx_detect_%d_%d_%d_%d",gr,cr,sl,ch);
	    cout << "Foolish event detected: detector not existing" << endl;
	    cout << "group : " << (int)gr << endl;
	    cout << "slot : " << (int)sl << endl;
	    cout << "crate : " << (int)cr << endl;
	    cout << "service : " << (int)ser << endl;
	    cout << "marker : " << (int)mark << endl;
	    cout << "channel : " << (int)ch << endl;
	    cout << "It is the event number : " << nbrevent2 << endl;
	    cout << "Of the " << nbrbuffer << "e buffer" << endl;
	    nbreventfous++;
	  }
      }// End of if(ser == 0)
				
	      
				
      // Read the next word to able to check if it is a physical hit or not
      if (narval.good()) narval.read(word1,2);
      if (narval.good()) narval.read(word2,2);
	      
      // In case of incomplete event, eof is reached unexpectedly
      if (narval.eof())
	{
	  cout << "Irregular end of file, incomplete event" << endl;
	  word1[0]=0xff;
	  word1[1]=0xff;
	  word2[0]=0xff;
	  word2[1]=0xff;                              // set marker for eof
	}// End of if (narval.eof())
    }// End of while ((word(swap,word1[0],word1[1])==0xffff) && (word(swap,word2[0],word2[1])==0x8))
			
    // If file contains loose zeroes in the middle, mark it as an internal header
    if((word(swap,word1[0],word1[1])==0x0) && (word(swap,word2[0],word2[1])==0x0))
      {
	cout << "Irregular file structure,loose zeroes found at "<< narval.tellg()-(streampos)4<< endl;
	// narval.seekg(0,ios::end);                  // move to eof without provoking eof yet
	// narval.read(word2,2);                      // actually provoke eof
	word1[0]=0xff;
	word1[1]=0xff;
	word2[0]=0xff;
	word2[1]=0xff;                                 // set marker for eof
      }// End of if((word(swap,word1[0],word1[1])==0x0) && (word(swap,word2[0],word2[1])==0x0))
			
    // This is an internal header or regular eof or eof incomplete event or loose zeroes
    else if((word(swap,word1[0],word1[1])==0xffff) && (word(swap,word2[0],word2[1])==0xffff))
      {
	if (narval.good()) narval.read(word1, 2);
	if (narval.good()) narval.read(word2, 2);
				
	// execute the loop as long as we don't encounter a detector event
	//cout<<" apres "<<n<<" evenements on cherche de nouveaux evenements physiques"<<endl ;
	while(!((word(swap,word1[0],word1[1])==0xffff) && ( (word(swap,word2[0],word2[1])==0x8) || (word(swap,word2[0],word2[1])==0xd))))
	  {
	    //	  cout<<n<<endl ; //ASP
	    narval.seekg(-2,ios::cur);
	    //ASP
	    //  cout<<narval.fail()<< " "<<narval.good()<< " "<<narval.bad()<< " "<<narval.eof()<<" "<<n<< endl ;
	    if(narval.good()) narval.read(word1,2);
	    if(narval.good()) narval.read(word2,2);
	    if(narval.eof())
	      {
		cout << "End of file detected" << endl;
							
		if(swap == 0)
		  {
		    // not very elegant but we need to escape from the while loop
		    word1[0]=0xff;
		    word1[1]=0xff;
		    word2[0]=0x8;
		    word2[1]=0x0;
		  }
		if(swap == 1)
		  {
		    // not very elegant but we need to escape from the while loop
		    word1[0]=0xff;
		    word1[1]=0xff;
		    word2[0]=0x0;
		    word2[1]=0x8;
		  }
	      }// End of if(narval.eof())
	  }// End of while(!((word(swap,word1[0],word1[1])==0xffff) && ( (word(swap,word2[0],word2[1])==0x8) || (word(swap,word2[0],word2[1])==0xd))))
      }// End of else if((word(swap,word1[0],word1[1])==0xffff) && (word(swap,word2[0],word2[1])==0xffff))
			
    else
      {
	cout << "Error, irregular event structure: corrupted event "<< endl;
	cout << "Searching for the next possible event" << endl;
	if (narval.good()) narval.read(word1, 2);
	if (narval.good()) narval.read(word2, 2);
				
	// execute the loop as long as we don't encounter a detector event
	//cout<<" apres "<<n<<" evenements on cherche de nouveaux evenements physiques"<<endl ;
	while(!((word(swap,word1[0],word1[1])==0xffff) && ( (word(swap,word2[0],word2[1])==0x8) || (word(swap,word2[0],word2[1])==0xd))))
	  {
	    //	  cout<<n<<endl ; //ASP
	    narval.seekg(-2,ios::cur);
	    //ASP
	    //  cout<<narval.fail()<< " "<<narval.good()<< " "<<narval.bad()<< " "<<narval.eof()<<" "<<n<< endl ;
	    if(narval.good()) narval.read(word1,2);
	    if(narval.good()) narval.read(word2,2);
	    if(narval.eof())
	      {
		cout << "End of file detected" << endl;
						
		if(swap == 0)
		  {
		    // not very elegant but we need to escape from the while loop
		    word1[0]=0xff;
		    word1[1]=0xff;
		    word2[0]=0x8;
		    word2[1]=0x0;
		  }
		if(swap == 1)
		  {
		    // not very elegant but we need to escape from the while loop
		    word1[0]=0xff;
		    word1[1]=0xff;
		    word2[0]=0x0;
		    word2[1]=0x8;
		  }
	      }// End of if(narval.eof())
	  }// End of while(!((word(swap,word1[0],word1[1])==0xffff) && ( (word(swap,word2[0],word2[1])==0x8) || (word(swap,word2[0],word2[1])==0xd))))
	//return 2;
      }
  }// End of while for eof
	
  // Ending of the conversion procedure
  if(narval.eof()){
    
		
    //cout << "End of file detected ..." <<endl;
    cout << "Saving last events and tree " << endl;
    // Organization of the last buffer that might have a size < buffersize
    buffersize = nbrevent2;
		
    tri_buffers(tampon_tm, buffer_tm, tampon_enrj,buffer_enrj,tampon_marker,buffer_marker,tampon_idx, buffer_idx, buffersize, nbrdetect);
		
		
    // I increase the number of buffer
    nbrbuffer++;
		
    // Now the buffer is sorted I will be able to store it in the TTree
    for(int ll = 0; ll < buffersize; ll++){
	
      // I get the information back from the arrays
      index = buffer_idx[ll];
      marker = buffer_marker[ll];
      enrj = buffer_enrj[ll];
      tm = buffer_tm[ll];
      if(buffer_tm[ll] < 1.e-8) tm = 0;
			
      nbrcoup_detect_test[index]++;
			
      // I fill the tree
      //if(tm !=0 ) oak -> Fill();
      if(tm != 0 && (index < nbrdetect + 1) && (index != 0)){
	oak -> Fill();
	nbrcoup_detect_saved[index]++;
      }
    }
		
    // I have to store the "tampon"
    for(int ll = 0; ll < nbrdetect+1; ll++){
	
      // I get the information back from the "tampon" array
      index = tampon_idx[ll];
      marker = tampon_marker[ll];
      enrj = tampon_enrj[ll];
      tm = tampon_tm[ll];
			
      nbrcoup_detect_test[index]++;
			
      // I fill the tree with these data
      //oak -> Fill();
      if(tm != 0 && (index < nbrdetect + 1) && (index != 0)){
	    
	oak -> Fill();
	nbrcoup_detect_saved[index]++;
      }
    }
	  
		
		
    // Writing the tree in the root file to be sure the root file is well closed
    cout << "Saving the tree in the file" << endl;
    oak -> Write();
		
    //Coucou(2);
	  
    // I clear the memory of all the table I've created
    //       delete tampon_tm;
    //       delete tampon_enrj;
    //       delete tampon_marker;
    //       delete tampon_idx;
    //       delete buffer_marker_coden;
    //       delete buffer_enrj_coden;
    //       delete buffer_tm_coden;
    //       delete buffer_idx_coden;
    //       delete buffer_marker;
    //       delete buffer_idx;
    //       delete buffer_enrj;
    //       delete buffer_tm;


                
		
		
    // Closing of all the file used during the conversion process
    narval.close();
    outputfile.Close();
	
     // Writting of a summary of the conversion process
    cout << "This makes "<< codingenable << " coden read in the file" << endl;
    cout << "This makes "<< n-codingenable << " hits read in the file"<<endl;
    cout << "This makes "<< n-nbrevent << " service hits read in the file"<<endl;
    cout << "This makes "<< nbreventfous << " completely foolish events read in the file"<<endl;
    cout << "This makes "<< nbrdesordre << " hits that were not in order (in time)"<<endl;
    cout << "This makes "<< nbrevent << " physical hits read in the file"<<endl;
    Long64_t totalread(0);
    Long64_t totalsaved(0);
    for(int i = 1; i < nbrdetect+1; i++){
	
      cout << "This makes " << nbrcoup_detect[i] << " physical events read for the detector " << i << endl;
      cout << "And " << nbrcoup_detect_test[i] << " event after sorting" << endl;
      cout << "And " << nbrcoup_detect_saved[i] << " saved for the same detector" << endl;
      totalread += nbrcoup_detect[i];
      totalsaved += nbrcoup_detect_saved[i];
    }
    cout << "This make " << totalread << " physical event read in the file" << endl;
    cout << "And " << totalsaved << " events saved in the memory" << endl;
    cout << "Thus a difference of " << totalread-totalsaved << endl;
    cout << endl;
    

    delete buffer_marker;
//Coucou(1);
    delete buffer_enrj;
//Coucou(2);
    delete buffer_tm;
//Coucou(3);
    delete buffer_idx;
//Coucou(4);
    //delete buffer_marker_coden;
//Coucou(5);
    delete buffer_enrj_coden;
//Coucou(6);
    delete buffer_tm_coden;
//Coucou(7);
    delete buffer_idx_coden;
//Coucou(8);
    //delete tampon_tm;
//Coucou(9);    
	//delete tampon_enrj;
//Coucou(10);
    //delete tampon_marker; 
//Coucou(11);
    //delete tampon_idx;
//Coucou(12);
    delete nbrcoup_detect;
//Coucou(13);
    delete nbrcoup_detect_saved;
//Coucou(14);
    delete nbrcoup_detect_test;
//Coucou(15);
    delete word1;
//Coucou(16);
    delete word2;
//Coucou(17);
    delete bf16;
//Coucou(18);

  }// End of if(narval.eof())
	
  else{
    
    cout << "Error exiting program" << endl; 
    return 0;
  } 
  // Return an int because it is a function
  return 1;
}
