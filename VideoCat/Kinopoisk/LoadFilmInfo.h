#pragma once

class CollectionDB;
class EntryHandle;

void LoadFilmsInfo( CollectionDB * cdb, const EntryHandle & rootHandle );

void LoadSeriesInfo( CollectionDB * cdb, const EntryHandle & rootHandle );

int MarkAsUserDefinedInfo( CollectionDB * cdb, const EntryHandle & rootHandle );
