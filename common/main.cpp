#include "./include/structures.h"

int main(int argc, char *argv[]) {
    SymbolList *list =
            new SymbolList(
                    "a", new SymbolList(
                            "b", new SymbolList("c")));
    list->log();
    delete list;
    return 0;
}