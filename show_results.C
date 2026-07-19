void show_results(){
	
	TTree * trees[4] ;

	string filename[4] = {"pieno120mm.root", "pieno90mm.root", "pieno30mm.root", "pieno15mm.root"} ;

	for (int i=0; i<4; i++) {
		TFile * f = new TFile(filename[i].c_str());
		trees[i] = (TTree *) f->Get("t");
		trees[i]->SetLineColor(i+1);
	}

	trees[0]->Draw("Edep","Edep>0");
	trees[1]->Draw("Edep","Edep>0", "same");
	trees[2]->Draw("Edep","Edep>0","same");
	trees[3]->Draw("Edep","Edep>0","same");

}