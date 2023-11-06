#pragma once
void* GetGSmem(int memidx);

// Display buffer modes
void writeTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void readTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

void writeTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void readTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

void writeTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void readTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

// Z Buffer modes
void readTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void writeTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

void readTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void writeTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

void readTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void writeTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

// Texture modes
void writeTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void readTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);

void writeTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
void readTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, int memidx, void* data);
