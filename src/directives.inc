enum PPDirective {
  PPNotADirective,
  PPif /*#if*/,
  PPifdef /*#ifdef*/,
  PPifndef /*#ifndef*/,
  PPelse /*#else*/,
  PPelseif /*#elseif*/,
  PPendif /*#endif*/,
  PPundef /*#undef*/,
  PPpragma /*#pragma*/,
  PPline /*#line*/,
  PPinclude /*#include*/,
  PPwarning /*#warning*/,
  PPerror /*#error*/
};
enum PPDirective getDirective(const char *s){
	switch(*s++){
		case 'i':
		switch(*s++){
			case 'f':
			switch(*s++){
				case '\0': return PPif;
				case 'd':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'f':
						switch(*s++){
							case '\0': return PPifdef;
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				case 'n':
				switch(*s++){
					case 'd':
					switch(*s++){
						case 'e':
						switch(*s++){
							case 'f':
							switch(*s++){
								case '\0': return PPifndef;
								default: return PPNotADirective;
							}
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			case 'n':
			switch(*s++){
				case 'c':
				switch(*s++){
					case 'l':
					switch(*s++){
						case 'u':
						switch(*s++){
							case 'd':
							switch(*s++){
								case 'e':
								switch(*s++){
									case '\0': return PPinclude;
									default: return PPNotADirective;
								}
								default: return PPNotADirective;
							}
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			default: return PPNotADirective;
		}
		case 'e':
		switch(*s++){
			case 'l':
			switch(*s++){
				case 's':
				switch(*s++){
					case 'e':
					switch(*s++){
						case '\0': return PPelse;
						case 'i':
						switch(*s++){
							case 'f':
							switch(*s++){
								case '\0': return PPelseif;
								default: return PPNotADirective;
							}
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			case 'n':
			switch(*s++){
				case 'd':
				switch(*s++){
					case 'i':
					switch(*s++){
						case 'f':
						switch(*s++){
							case '\0': return PPendif;
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			case 'r':
			switch(*s++){
				case 'r':
				switch(*s++){
					case 'o':
					switch(*s++){
						case 'r':
						switch(*s++){
							case '\0': return PPerror;
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			default: return PPNotADirective;
		}
		case 'u':
		switch(*s++){
			case 'n':
			switch(*s++){
				case 'd':
				switch(*s++){
					case 'e':
					switch(*s++){
						case 'f':
						switch(*s++){
							case '\0': return PPundef;
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			default: return PPNotADirective;
		}
		case 'p':
		switch(*s++){
			case 'r':
			switch(*s++){
				case 'a':
				switch(*s++){
					case 'g':
					switch(*s++){
						case 'm':
						switch(*s++){
							case 'a':
							switch(*s++){
								case '\0': return PPpragma;
								default: return PPNotADirective;
							}
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			default: return PPNotADirective;
		}
		case 'l':
		switch(*s++){
			case 'i':
			switch(*s++){
				case 'n':
				switch(*s++){
					case 'e':
					switch(*s++){
						case '\0': return PPline;
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			default: return PPNotADirective;
		}
		case 'w':
		switch(*s++){
			case 'a':
			switch(*s++){
				case 'r':
				switch(*s++){
					case 'n':
					switch(*s++){
						case 'i':
						switch(*s++){
							case 'n':
							switch(*s++){
								case 'g':
								switch(*s++){
									case '\0': return PPwarning;
									default: return PPNotADirective;
								}
								default: return PPNotADirective;
							}
							default: return PPNotADirective;
						}
						default: return PPNotADirective;
					}
					default: return PPNotADirective;
				}
				default: return PPNotADirective;
			}
			default: return PPNotADirective;
		}
		default: return PPNotADirective;
	}
}
